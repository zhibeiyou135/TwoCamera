#include "DetectionSessionManager.h"
#include "ConfigManager.h"
#include "RecordingConfig.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QStandardPaths>

DetectionSessionManager* DetectionSessionManager::instance = nullptr;

DetectionSessionManager* DetectionSessionManager::getInstance() {
    if (!instance) {
        instance = new DetectionSessionManager();
    }
    return instance;
}

DetectionSessionManager::DetectionSessionManager() {
    // 异步加载配置，避免阻塞构造函数
    QtConcurrent::run([this]() {
        loadConfiguration();
        configLoaded.store(true);
    });
}

void DetectionSessionManager::DetectionSessionConfig::loadFromJson(const QJsonObject& detectionObj) {
    if (detectionObj.contains("saveDetectionImages") && detectionObj["saveDetectionImages"].isBool()) {
        saveDetectionImages = detectionObj["saveDetectionImages"].toBool();
    }
    
    if (detectionObj.contains("detectionImagePath") && detectionObj["detectionImagePath"].isString()) {
        detectionImagePath = detectionObj["detectionImagePath"].toString();
    }
    
    if (detectionObj.contains("createSessionFolders") && detectionObj["createSessionFolders"].isBool()) {
        createSessionFolders = detectionObj["createSessionFolders"].toBool();
    }
    

}

void DetectionSessionManager::loadConfiguration() {
    try {
        QString configPath = ConfigManager::getInstance().getConfigPath();
        QFile configFile(configPath);
        
        if (!configFile.open(QIODevice::ReadOnly)) {
            return;
        }
        
        QByteArray jsonData = configFile.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        
        if (!doc.isObject()) {
            return;
        }
        
        QJsonObject rootObj = doc.object();
        if (!rootObj.contains("detection") || !rootObj["detection"].isObject()) {
            return;
        }
        
        QJsonObject detectionObj = rootObj["detection"].toObject();
        config.loadFromJson(detectionObj);
        
    } catch (const std::exception& e) {
        qDebug() << "加载检测会话配置时发生异常:" << e.what();
    }
}

void DetectionSessionManager::startDetectionSession() {
    QMutexLocker locker(&sessionMutex);

    if (sessionActive.load()) {
        // 检查当前会话是否仍然有效
        if (!currentSessionPath.isEmpty() && QDir(currentSessionPath).exists()) {
            qDebug() << "检测会话已经处于活动状态且路径有效，复用现有会话:" << currentSessionPath;
            return;
        } else {
            qDebug() << "检测会话活动但路径无效，重新创建会话";
            sessionActive.store(false);
            currentSessionPath.clear();
        }
    }

    qDebug() << "启动检测会话...";
    
    // 检查是否有活动的录制会话
    auto recordingConfig = RecordingConfig::getInstance();
    QString recordingSessionPath = recordingConfig->getCurrentSessionPath();

    if (!recordingSessionPath.isEmpty()) {
        // 有录制会话时，直接使用录制会话路径，不创建新文件夹
        currentSessionPath = recordingSessionPath;
        qDebug() << "检测会话使用现有录制会话路径:" << currentSessionPath;
    } else if (config.createSessionFolders) {
        // 没有录制会话且配置允许时，创建独立检测会话文件夹
        currentSessionPath = createSessionFolder();
        // 异步创建检测文件夹，避免阻塞
        QtConcurrent::run([this, sessionPath = currentSessionPath]() {
            ensureDetectionFolders(sessionPath);
        });
    } else {
        // 不创建会话文件夹，使用录制配置的基础路径
        currentSessionPath = recordingConfig->getBasePath();

        // 异步确保检测结果目录存在
        QString detectionPath = currentSessionPath + "/" + config.detectionImagePath;
        QtConcurrent::run([detectionPath]() {
            QDir().mkpath(detectionPath + "/dv");
            QDir().mkpath(detectionPath + "/dvs");
        });
    }
    
    sessionStartTime = QDateTime::currentDateTime();
    sessionActive.store(true);

    // 清空路径缓存，因为会话路径已改变
    {
        QMutexLocker cacheLocker(&pathCacheMutex);
        pathCache.clear();
    }



    qDebug() << "检测会话启动完成，会话路径:" << currentSessionPath;
}

void DetectionSessionManager::endDetectionSession() {
    QMutexLocker locker(&sessionMutex);
    
    if (!sessionActive.load()) {
        qDebug() << "检测会话未处于活动状态";
        return;
    }
    
    qDebug() << "结束检测会话，会话持续时间:" 
             << sessionStartTime.secsTo(QDateTime::currentDateTime()) << "秒";
    
    sessionActive.store(false);
    currentSessionPath.clear();
}

void DetectionSessionManager::resetDetectionSession() {
    qDebug() << "重置检测会话";

    // 先结束当前会话（如果存在）
    {
        QMutexLocker locker(&sessionMutex);
        if (sessionActive.load()) {
            qDebug() << "结束当前检测会话，会话持续时间:"
                     << sessionStartTime.secsTo(QDateTime::currentDateTime()) << "秒";
            sessionActive.store(false);
            currentSessionPath.clear();

            // 清空路径缓存
            {
                QMutexLocker cacheLocker(&pathCacheMutex);
                pathCache.clear();
            }


        }
    }

    // 启动新会话（在新的锁作用域中）
    startDetectionSession();
}

bool DetectionSessionManager::isSessionActive() const {
    return sessionActive.load();
}

QString DetectionSessionManager::getDetectionResultPath(const QString& cameraType) {
    // 等待配置加载完成（非阻塞检查）
    if (!configLoaded.load()) {
        // 配置未加载完成，使用默认配置
        if (!config.saveDetectionImages) {
            return QString();
        }
    } else {
        // 配置已加载，检查是否保存检测图片
        if (!config.saveDetectionImages) {
            return QString();
        }
    }

    // 检查路径缓存 - 包含会话状态的复合键
    QString sessionId = QString::number(sessionStartTime.toMSecsSinceEpoch());
    QString cacheKey = cameraType.toLower() + "_" + sessionId;
    {
        QMutexLocker cacheLocker(&pathCacheMutex);
        if (pathCache.contains(cacheKey)) {
            QString cachedPath = pathCache.value(cacheKey);
            // 验证缓存路径仍然有效
            if (QDir(cachedPath).exists() || QDir().mkpath(cachedPath)) {
                return cachedPath;
            } else {
                // 缓存路径无效，移除缓存
                pathCache.remove(cacheKey);
            }
        }
    }

    QMutexLocker locker(&sessionMutex);

    // 如果会话未激活，启动会话
    if (!sessionActive.load()) {
        locker.unlock();
        startDetectionSession();
        locker.relock();
    }

    // 构建检测结果路径
    QString detectionPath;
    if (config.createSessionFolders) {
        detectionPath = currentSessionPath + "/" + config.detectionImagePath + "/" + cameraType.toLower();
    } else {
        // 不创建会话文件夹时，检查是否有录制会话
        auto recordingConfig = RecordingConfig::getInstance();
        QString recordingSessionPath = recordingConfig->getCurrentSessionPath();

        if (!recordingSessionPath.isEmpty()) {
            // 使用录制会话路径
            detectionPath = recordingSessionPath + "/" + config.detectionImagePath + "/" + cameraType.toLower();
        } else {
            // 使用基础路径
            detectionPath = currentSessionPath + "/" + config.detectionImagePath + "/" + cameraType.toLower();
        }
    }

    // 异步创建目录，避免阻塞UI
    QtConcurrent::run([detectionPath]() {
        QDir().mkpath(detectionPath);
    });

    // 缓存路径
    {
        QMutexLocker cacheLocker(&pathCacheMutex);
        pathCache.insert(cacheKey, detectionPath);
    }

    return detectionPath;
}

QString DetectionSessionManager::getCurrentSessionPath() const {
    QMutexLocker locker(&sessionMutex);
    return currentSessionPath;
}



bool DetectionSessionManager::isCurrentSessionValid() const {
    QMutexLocker locker(&sessionMutex);
    return sessionActive.load() && !currentSessionPath.isEmpty() && QDir(currentSessionPath).exists();
}

QString DetectionSessionManager::createSessionFolder() {
    // 优先使用录制配置的基础路径
    auto recordingConfig = RecordingConfig::getInstance();
    QString basePath = recordingConfig->getBasePath();

    // 检查是否有活动的录制会话
    QString recordingSessionPath = recordingConfig->getCurrentSessionPath();
    if (!recordingSessionPath.isEmpty()) {
        // 如果有录制会话，直接使用录制会话路径，不创建新的检测会话文件夹
        qDebug() << "使用现有录制会话路径:" << recordingSessionPath;
        return recordingSessionPath;
    }

    // 创建独立的检测会话文件夹（仅在没有录制会话时）
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
    QString sessionPath = QDir(basePath).absoluteFilePath(timestamp + "_detection_session");

    // 异步创建主文件夹，避免阻塞UI
    QtConcurrent::run([sessionPath, this]() {
        if (!QDir().mkpath(sessionPath)) {
            qDebug() << "警告: 无法创建检测会话文件夹:" << sessionPath;
            // 回退到文档目录
            QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
            QString fallbackPath = QDir(documentsPath).absoluteFilePath("detection_sessions/" +
                QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss"));
            QDir().mkpath(fallbackPath);
        }
    });

    return sessionPath;
}

void DetectionSessionManager::ensureDetectionFolders(const QString& sessionPath) {
    QString detectionBasePath = sessionPath + "/" + config.detectionImagePath;
    
    // 创建检测结果基础目录
    QDir().mkpath(detectionBasePath);
    
    // 创建DV和DVS子目录
    QDir().mkpath(detectionBasePath + "/dv");
    QDir().mkpath(detectionBasePath + "/dvs");
    
    qDebug() << "检测结果文件夹创建完成:" << detectionBasePath;
}
