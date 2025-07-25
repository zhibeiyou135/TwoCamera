#ifndef TWOCAMERA_RECORDINGCONFIG_H
#define TWOCAMERA_RECORDINGCONFIG_H

#include <QString>
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <atomic>
#include <mutex>

class RecordingConfig {
public:
    static RecordingConfig* getInstance() {
        static RecordingConfig instance;
        return &instance;
    }

    // 基础路径设置
    void setBasePath(const QString& path) { 
        std::lock_guard<std::mutex> lock(configMutex);
        basePath = path; 
    }
    QString getBasePath() const { 
        std::lock_guard<std::mutex> lock(configMutex);
        return basePath; 
    }

    // 保存选项设置
    void setSaveDVOriginal(bool enable) { saveDVOriginal.store(enable); }
    bool getSaveDVOriginal() const { return saveDVOriginal.load(); }

    void setSaveDVCropped(bool enable) { saveDVCropped.store(enable); }
    bool getSaveDVCropped() const { return saveDVCropped.load(); }

    void setSaveDVSImages(bool enable) { saveDVSImages.store(enable); }
    bool getSaveDVSImages() const { return saveDVSImages.load(); }

    void setSaveDVSRaw(bool enable) { saveDVSRaw.store(enable); }
    bool getSaveDVSRaw() const { return saveDVSRaw.load(); }

    void setCreateTimestampFolders(bool enable) { createTimestampFolders.store(enable); }
    bool getCreateTimestampFolders() const { return createTimestampFolders.load(); }

    // 创建录制会话路径
    QString createRecordingSession(const QString& sessionSuffix = "") {
        std::lock_guard<std::mutex> lock(configMutex);
        
        if (!createTimestampFolders.load()) {
            return basePath;
        }

        QString sessionPath;
        
        // 检查是否是自动录制（包含auto_前缀）
        if (sessionSuffix.startsWith("auto_")) {
            // 解析盘片编号和位置信息
            QString suffix = sessionSuffix.mid(5); // 去掉"auto_"前缀
            QStringList parts = suffix.split("_");
            
            if (parts.size() >= 2) {
                QString discNumber = parts[0];
                QString position = parts[1];

                // 为自动录制创建正确的文件夹结构：数字文件夹/位置_时间戳
                QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
                QString discFolder = discNumber; // 直接使用数字作为文件夹名
                QString positionFolder = QString("%1_%2").arg(position, timestamp);

                currentSessionPath = QDir(basePath).absoluteFilePath(discFolder + "/" + positionFolder);
            } else {
                // 如果解析失败，使用默认方式
                QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
                currentSessionPath = QDir(basePath).absoluteFilePath(timestamp + "_" + sessionSuffix);
            }
        } else {
            // 普通录制会话，创建时间戳文件夹名
            QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
            if (!sessionSuffix.isEmpty()) {
                timestamp += "_" + sessionSuffix;
            }

            currentSessionPath = QDir(basePath).absoluteFilePath(timestamp);
        }
        
        // 创建主录制文件夹
        QDir().mkpath(currentSessionPath);

        // 创建子文件夹
        if (saveDVOriginal.load()) {
            QDir().mkpath(currentSessionPath + "/dv_original");
        }
        if (saveDVCropped.load()) {
            QDir().mkpath(currentSessionPath + "/dv_cropped");
        }
        if (saveDVSImages.load()) {
            QDir().mkpath(currentSessionPath + "/dvs_images");
        }
        if (saveDVSRaw.load()) {
            QDir().mkpath(currentSessionPath + "/dvs_raw");
        }
        
        // 如果是检测相关的会话，创建检测结果目录
        if (sessionSuffix.contains("detect", Qt::CaseInsensitive)) {
            QDir().mkpath(currentSessionPath + "/detection_results");
            QDir().mkpath(currentSessionPath + "/detection_results/dv");
            QDir().mkpath(currentSessionPath + "/detection_results/dvs");
        }
        return currentSessionPath;
    }

    // 获取各种路径
    QString getCurrentSessionPath() const {
        std::lock_guard<std::mutex> lock(configMutex);
        return currentSessionPath;
    }

    QString getDVOriginalPath() const {
        std::lock_guard<std::mutex> lock(configMutex);
        return currentSessionPath + "/dv_original";
    }

    QString getDVCroppedPath() const {
        std::lock_guard<std::mutex> lock(configMutex);
        return currentSessionPath + "/dv_cropped";
    }

    QString getDVSImagesPath() const {
        std::lock_guard<std::mutex> lock(configMutex);
        return currentSessionPath + "/dvs_images";
    }

    QString getDVSRawPath() const {
        std::lock_guard<std::mutex> lock(configMutex);
        return currentSessionPath + "/dvs_raw";
    }

    // 清理当前会话
    void clearCurrentSession() {
        std::lock_guard<std::mutex> lock(configMutex);
        currentSessionPath.clear();
    }

private:
    RecordingConfig() : 
        basePath("/home/pe/yxl/TwoCamera_v6detect/recordings"),
        saveDVOriginal(true),
        saveDVCropped(true),
        saveDVSImages(true),
        saveDVSRaw(true),
        createTimestampFolders(true) {}

    mutable std::mutex configMutex;
    QString basePath;
    QString currentSessionPath;
    std::atomic<bool> saveDVOriginal;
    std::atomic<bool> saveDVCropped;
    std::atomic<bool> saveDVSImages;
    std::atomic<bool> saveDVSRaw;
    std::atomic<bool> createTimestampFolders;
};

// 声明JSON加载函数
void loadRecordingConfigFromJson(const QString& jsonPath);

#endif // TWOCAMERA_RECORDINGCONFIG_H 