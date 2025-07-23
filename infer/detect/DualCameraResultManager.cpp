#include "DualCameraResultManager.h"
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

DualCameraResultManager::~DualCameraResultManager() {
    if (processTimer) {
        processTimer->stop();
        processTimer->deleteLater();
        processTimer = nullptr;
    }
    qDebug() << "DualCameraResultManager析构函数完成";
}

void DualCameraResultManager::initialize(const QString &outputFile) {
    // 设置输出文件路径
    if (outputFile.isEmpty()) {
        QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        outputFilePath = QDir(documentsPath).filePath("detection_results.txt");
    } else {
        outputFilePath = outputFile;
    }
    
    // 初始化定时器
    processTimer = new QTimer(this);
    connect(processTimer, &QTimer::timeout, this, &DualCameraResultManager::onProcessResults);
    processTimer->start(100); // 每100ms处理一次结果
    
    qDebug() << "双摄像头结果管理器初始化完成，输出文件:" << outputFilePath;
}

void DualCameraResultManager::addDVResult(const QString &detectionInfo, bool hasDetection, 
                                         const QString &className, float confidence) {
    QMutexLocker locker(&resultMutex);
    
    DetectionResult result;
    result.cameraType = "DV";
    result.detectionInfo = detectionInfo;
    result.timestamp = QDateTime::currentDateTime();
    result.hasDetection = hasDetection;
    result.className = className;
    result.confidence = confidence;
    
    dvResults.push_back(result);
    
    // 限制队列大小
    if (dvResults.size() > maxQueueSize) {
        dvResults.pop_front();
    }
    
    qDebug() << "添加DV检测结果:" << detectionInfo << "时间:" << result.timestamp.toString();
}

void DualCameraResultManager::addDVSResult(const QString &detectionInfo, bool hasDetection, 
                                          const QString &className, float confidence) {
    QMutexLocker locker(&resultMutex);
    
    DetectionResult result;
    result.cameraType = "DVS";
    result.detectionInfo = detectionInfo;
    result.timestamp = QDateTime::currentDateTime();
    result.hasDetection = hasDetection;
    result.className = className;
    result.confidence = confidence;
    
    dvsResults.push_back(result);
    
    // 限制队列大小
    if (dvsResults.size() > maxQueueSize) {
        dvsResults.pop_front();
    }
    
    qDebug() << "添加DVS检测结果:" << detectionInfo << "时间:" << result.timestamp.toString();
}

void DualCameraResultManager::onProcessResults() {
    QMutexLocker locker(&resultMutex);
    processMatchedResults();
    cleanupOldResults();
}

void DualCameraResultManager::processMatchedResults() {
    // 遍历DV结果，寻找时间匹配的DVS结果
    for (auto dvIt = dvResults.begin(); dvIt != dvResults.end(); ) {
        bool matched = false;
        
        for (auto dvsIt = dvsResults.begin(); dvsIt != dvsResults.end(); ) {
            // 计算时间差
            qint64 timeDiff = qAbs(dvIt->timestamp.msecsTo(dvsIt->timestamp));
            
            if (timeDiff <= timeWindowMs) {
                // 找到匹配的结果对
                QString finalDecision = generateFinalDecision(*dvIt, *dvsIt);
                QDateTime avgTime = QDateTime::fromMSecsSinceEpoch(
                    (dvIt->timestamp.toMSecsSinceEpoch() + dvsIt->timestamp.toMSecsSinceEpoch()) / 2
                );
                
                // 写入文件并发出信号
                writeFinalDecision(finalDecision, avgTime);
                emit finalDecisionMade(finalDecision, avgTime);
                
                // 删除已处理的结果
                dvsIt = dvsResults.erase(dvsIt);
                matched = true;
                break;
            } else {
                ++dvsIt;
            }
        }
        
        if (matched) {
            dvIt = dvResults.erase(dvIt);
        } else {
            ++dvIt;
        }
    }
}

void DualCameraResultManager::cleanupOldResults() {
    QDateTime currentTime = QDateTime::currentDateTime();
    qint64 thresholdMs = timeWindowMs * 10; // 保留时间窗口的10倍时间
    
    // 清理过期的DV结果
    while (!dvResults.empty() && 
           currentTime.msecsTo(dvResults.front().timestamp) > thresholdMs) {
        dvResults.pop_front();
    }
    
    // 清理过期的DVS结果
    while (!dvsResults.empty() && 
           currentTime.msecsTo(dvsResults.front().timestamp) > thresholdMs) {
        dvsResults.pop_front();
    }
}

QString DualCameraResultManager::generateFinalDecision(const DetectionResult &dvResult, 
                                                      const DetectionResult &dvsResult) {
    QString decision;
    
    // 如果两个摄像头都有检测结果
    if (dvResult.hasDetection && dvsResult.hasDetection) {
        if (dvResult.className == dvsResult.className) {
            // 类别一致，取较高的置信度
            float avgConfidence = (dvResult.confidence + dvsResult.confidence) / 2.0f;
            decision = QString("一致检测: %1 (平均置信度: %2)")
                      .arg(dvResult.className)
                      .arg(avgConfidence, 0, 'f', 2);
        } else {
            // 类别不一致，需要进一步判断
            if (dvResult.confidence > dvsResult.confidence) {
                decision = QString("冲突检测(DV优先): %1 (置信度: %2) vs %3 (置信度: %4)")
                          .arg(dvResult.className).arg(dvResult.confidence, 0, 'f', 2)
                          .arg(dvsResult.className).arg(dvsResult.confidence, 0, 'f', 2);
            } else {
                decision = QString("冲突检测(DVS优先): %1 (置信度: %2) vs %3 (置信度: %4)")
                          .arg(dvsResult.className).arg(dvsResult.confidence, 0, 'f', 2)
                          .arg(dvResult.className).arg(dvResult.confidence, 0, 'f', 2);
            }
        }
    } else if (dvResult.hasDetection) {
        // 只有DV有检测结果
        decision = QString("DV单独检测: %1 (置信度: %2)")
                  .arg(dvResult.className).arg(dvResult.confidence, 0, 'f', 2);
    } else if (dvsResult.hasDetection) {
        // 只有DVS有检测结果
        decision = QString("DVS单独检测: %1 (置信度: %2)")
                  .arg(dvsResult.className).arg(dvsResult.confidence, 0, 'f', 2);
    } else {
        // 两个摄像头都没有检测到目标
        decision = "无检测结果";
    }
    
    return decision;
}

void DualCameraResultManager::writeFinalDecision(const QString &decision, const QDateTime &timestamp) {
    QFile file(outputFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream(&file);
        stream << timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz") << " - " << decision << "\n";
        file.close();
    } else {
        qDebug() << "无法打开输出文件:" << outputFilePath;
    }
} 