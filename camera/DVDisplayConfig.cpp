#include "DVDisplayConfig.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>

// Load configuration from JSON file
void loadDVDisplayConfigFromJson(const QString& jsonPath) {
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
    if (!rootObj.contains("dvDisplay") || !rootObj["dvDisplay"].isObject()) {
        qDebug() << "Missing 'dvDisplay' section in configuration";
        return;
    }

    QJsonObject dvDisplayObj = rootObj["dvDisplay"].toObject();
    auto dvConfig = DVDisplayConfig::getInstance();

    // Display mode
    if (dvDisplayObj.contains("mode") && dvDisplayObj["mode"].isString()) {
        dvConfig->setDisplayMode(dvDisplayObj["mode"].toString().toStdString());
    }

    // Crop settings
    if (dvDisplayObj.contains("crop") && dvDisplayObj["crop"].isObject()) {
        QJsonObject cropObj = dvDisplayObj["crop"].toObject();
        
        if (cropObj.contains("enabled") && cropObj["enabled"].isBool()) {
            dvConfig->setCropEnabled(cropObj["enabled"].toBool());
        }
        
        if (cropObj.contains("x") && cropObj["x"].isDouble()) {
            dvConfig->setCropX(cropObj["x"].toInt());
        }
        
        if (cropObj.contains("y") && cropObj["y"].isDouble()) {
            dvConfig->setCropY(cropObj["y"].toInt());
        }
        
        if (cropObj.contains("width") && cropObj["width"].isDouble()) {
            dvConfig->setCropWidth(cropObj["width"].toInt());
        }
        
        if (cropObj.contains("height") && cropObj["height"].isDouble()) {
            dvConfig->setCropHeight(cropObj["height"].toInt());
        }
    }

    // Save settings
    if (dvDisplayObj.contains("save") && dvDisplayObj["save"].isObject()) {
        QJsonObject saveObj = dvDisplayObj["save"].toObject();
        
        if (saveObj.contains("enabled") && saveObj["enabled"].isBool()) {
            dvConfig->setSaveEnabled(saveObj["enabled"].toBool());
        }
        
        if (saveObj.contains("path") && saveObj["path"].isString()) {
            dvConfig->setSavePath(saveObj["path"].toString().toStdString());
        }
    }

    // Rotation settings
    if (dvDisplayObj.contains("rotation") && dvDisplayObj["rotation"].isObject()) {
        QJsonObject rotationObj = dvDisplayObj["rotation"].toObject();
        
        if (rotationObj.contains("enabled") && rotationObj["enabled"].isBool()) {
            dvConfig->setRotationEnabled(rotationObj["enabled"].toBool());
        }
        
        if (rotationObj.contains("angle") && rotationObj["angle"].isDouble()) {
            dvConfig->setRotationAngle(rotationObj["angle"].toDouble());
        }
        
        if (rotationObj.contains("border_mode") && rotationObj["border_mode"].isString()) {
            dvConfig->setRotationBorderMode(rotationObj["border_mode"].toString().toStdString());
        }
        
        if (rotationObj.contains("interpolation") && rotationObj["interpolation"].isString()) {
            dvConfig->setRotationInterpolation(rotationObj["interpolation"].toString().toStdString());
        }
        
        qDebug() << "Loaded rotation settings - enabled:" << dvConfig->getRotationEnabled()
                 << "angle:" << dvConfig->getRotationAngle() 
                 << "border_mode:" << QString::fromStdString(dvConfig->getRotationBorderMode())
                 << "interpolation:" << QString::fromStdString(dvConfig->getRotationInterpolation());
    }

    qDebug() << "Loaded DV display configuration from" << jsonPath;
} 