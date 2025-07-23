#include "RecordingConfig.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDebug>

// 从JSON文件加载录制配置
void loadRecordingConfigFromJson(const QString& jsonPath) {
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open JSON configuration file:" << jsonPath;
        return;
    }

    QByteArray jsonData = file.readAll();
    QJsonDocument document = QJsonDocument::fromJson(jsonData);
    
    if (document.isNull() || !document.isObject()) {
        qDebug() << "Invalid JSON format in configuration file:" << jsonPath;
        return;
    }

    QJsonObject rootObj = document.object();
    if (!rootObj.contains("recordingOptions") || !rootObj["recordingOptions"].isObject()) {
        qDebug() << "Missing 'recordingOptions' section in configuration, using defaults";
        return;
    }

    QJsonObject recordingObj = rootObj["recordingOptions"].toObject();
    auto recordingConfig = RecordingConfig::getInstance();

    // 基础路径
    if (recordingObj.contains("basePath") && recordingObj["basePath"].isString()) {
        recordingConfig->setBasePath(recordingObj["basePath"].toString());
        qDebug() << "Set recording base path to:" << recordingObj["basePath"].toString();
    }

    // 保存选项
    if (recordingObj.contains("saveDVOriginal") && recordingObj["saveDVOriginal"].isBool()) {
        recordingConfig->setSaveDVOriginal(recordingObj["saveDVOriginal"].toBool());
    }

    if (recordingObj.contains("saveDVCropped") && recordingObj["saveDVCropped"].isBool()) {
        recordingConfig->setSaveDVCropped(recordingObj["saveDVCropped"].toBool());
    }

    if (recordingObj.contains("saveDVSImages") && recordingObj["saveDVSImages"].isBool()) {
        recordingConfig->setSaveDVSImages(recordingObj["saveDVSImages"].toBool());
    }

    if (recordingObj.contains("saveDVSRaw") && recordingObj["saveDVSRaw"].isBool()) {
        recordingConfig->setSaveDVSRaw(recordingObj["saveDVSRaw"].toBool());
    }

    if (recordingObj.contains("createTimestampFolders") && recordingObj["createTimestampFolders"].isBool()) {
        recordingConfig->setCreateTimestampFolders(recordingObj["createTimestampFolders"].toBool());
    }

    qDebug() << "Loaded recording configuration from" << jsonPath;
    qDebug() << "Recording options:"
             << "\n  basePath:" << recordingConfig->getBasePath()
             << "\n  saveDVOriginal:" << recordingConfig->getSaveDVOriginal()
             << "\n  saveDVCropped:" << recordingConfig->getSaveDVCropped()
             << "\n  saveDVSImages:" << recordingConfig->getSaveDVSImages()
             << "\n  saveDVSRaw:" << recordingConfig->getSaveDVSRaw()
             << "\n  createTimestampFolders:" << recordingConfig->getCreateTimestampFolders();
} 