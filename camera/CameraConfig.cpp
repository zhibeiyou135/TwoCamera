//
// Created by pe on 2020/10/12.
//

#include "CameraConfig.h"
#include "camera/CameraCapture.h"
#include "dvs/DVSDataSource.h"
#include "infer/classification/ClassificationModule.h"
#include "infer/classification/EightModelPolicy.h"
#include "infer/classification/NiguangModelPolicy.h"
#include "infer/classification/NiguangModelPolicyPlus.h"
#include "infer/classification/TRTModel.h"
#include "infer/construct/ConstructModule.h"
#include "infer/construct/SingleDVSConstructModel.h"
#include "infer/construct/SingleDVSConstructUnetModelPolicy.h"
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>

void CameraConfig::loadConfigFile(const QString &file) {
  qDebug() << "load config file " << file;
  QFile io(file);
  if (!io.open(io.ReadOnly)) {
    qDebug() << "not found config file";
    return;
  }
  auto obj = QJsonDocument::fromJson(io.readAll()).object();
  auto keys = obj.keys();
  qDebug() <<keys;
  if (obj.contains("controls")) {
    auto controls = obj["controls"].toObject();
    if (controls.contains("dv")) {
      cameraControlConfig.dv = controls["dv"].toBool();
    }
    if (controls.contains("playback")) {
      cameraControlConfig.playback = controls["playback"].toBool();
      qDebug() << "set controls.playback";
    }
    if (controls.contains("denoise")) {
      cameraControlConfig.denoise = controls["denoise"].toBool();
      qDebug() << "set controls.denoise";
    }
    if (controls.contains("speed")) {
      cameraControlConfig.speed = controls["speed"].toBool();
      qDebug() << "set controls.speed";
    }
    if (controls.contains("overlap")) {
      cameraControlConfig.overlap = controls["overlap"].toBool();
      qDebug() << "set controls.overlap";
    }
    if (controls.contains("saveFormat")) {
      cameraControlConfig.saveFormat = controls["saveFormat"].toBool();
      qDebug() << "set controls.saveFormat";
    }
    if (controls.contains("modelSelect")) {
      cameraControlConfig.modelSelect = controls["modelSelect"].toBool();
      qDebug() << "set controls.modelSelect";
    }
    if (controls.contains("modelPolicySelect")) {
      cameraControlConfig.modelPolicySelect =
          controls["modelPolicySelect"].toBool();
      qDebug() << "set controls.modelPolicySelect";
    }
    if (controls.contains("saveOptions")) {
      cameraControlConfig.saveOptions = controls["saveOptions"].toBool();
      qDebug() << "set controls.saveOptions";
    }
    if (controls.contains("threshold")) {
      cameraControlConfig.threshold = controls["threshold"].toBool();
      qDebug() << "set controls.threshold";
    }
  }
  if (obj.contains("saveOptions")) {
    auto saveOptions = obj["saveOptions"].toObject();
    if (saveOptions.contains("saveDVSBin")) {
      DVSDataSource::getInstance()->setEnableSaveRaw(
          saveOptions["saveDVSBin"].toBool());
    }
    if (saveOptions.contains("saveDVSImg")) {
      DVSDataSource::getInstance()->setEnableSaveImg(
          saveOptions["saveDVSImg"].toBool());
    }
    if (saveOptions.contains("saveDV")) {
      CameraCapture::getInstance()->setAllowRecording(
          saveOptions["saveDV"].toBool());
    }
    if (saveOptions.contains("savePath")) {
      QString savePath = saveOptions["savePath"].toString();
      qDebug() << "Setting global save path to:" << savePath;
      QDir().mkpath(savePath);
      DVSDataSource::getInstance()->setSaveFolderPath(savePath);
      CameraCapture::getInstance()->setSavePath(savePath);
    }
  }
  if (obj.contains("fpn")) {
    auto f = obj["fpn"].toString();
    DVSDataSource::getInstance()->loadFpn(f);
  }
  /*
  {
    "construct":{
      "singleDVS":{
        "model":{
          "path":"model file path",
          "type":"model type factory function",
          "policy":"model policy type"
        }
      }
    }
  }
  */
  if (obj.contains("construct")) {
    auto construct = obj["construct"].toObject();
    if (construct.contains("singleDVS")) {
      cameraControlConfig.construct = true;
    }
  }
  /*
  {
    "classification":{
      "policy":"",
      "model":"",
      "path":""
    }
  }
  {
    "classification":true
  }
  */
  if (obj.contains("classification")) {
    //有classification选项，开启分类模块
    cameraControlConfig.classification = true;
    if (obj["classification"].isBool()) {
      cameraControlConfig.classification = obj["classification"].toBool();
    } else {
      auto classification = obj["classification"].toObject();
      cameraControlConfig.classification = true;
      setClassificationModelPolicy(classification["policy"].toString());
      setClassificationModel(classification["model"].toString());
      setClassificationModelFilePath(classification["path"].toString());
    }
  }
  if (obj.contains("fontPointSize")) {
    fontPointSize = obj["fontPointSize"].toInt();
  }
  if (obj.contains("dvsViewVerticalFlip")) {
    dvsViewVerticalFlip = obj["dvsViewVerticalFlip"].toBool();
  }
  if (obj.contains("dvsViewHorizontalFlip")) {
    auto r = obj["dvsViewHorizontalFlip"].toBool();
    dvsViewHorizontalFlip = r;
    qDebug() << "set dvsViewHorizontalFlip " << r;
  }
  if (obj.contains("dvViewVerticalFlip")) {
    dvViewVerticalFlip = obj["dvViewVerticalFlip"].toBool();
  }
  if (obj.contains("dvViewHorizontalFlip")) {
    dvViewHorizontalFlip = obj["dvViewHorizontalFlip"].toBool();
  }
  if (obj.contains("playbackOptions")) {
    auto playbackOptions = obj["playbackOptions"].toObject();
    
    if (playbackOptions.contains("enabled")) {
      playbackConfig.enabled = playbackOptions["enabled"].toBool();
      qDebug() << "Playback functionality enabled:" << playbackConfig.enabled;
    }
    
    if (playbackOptions.contains("mode")) {
      QString mode = playbackOptions["mode"].toString();
      playbackConfig.mode = mode;
      qDebug() << "Setting playback mode to:" << mode;
    }
    
    if (playbackOptions.contains("dvsEnabled")) {
      playbackConfig.dvsEnabled = playbackOptions["dvsEnabled"].toBool();
      qDebug() << "DVS playback enabled:" << playbackConfig.dvsEnabled;
    }
    
    if (playbackOptions.contains("dvEnabled")) {
      playbackConfig.dvEnabled = playbackOptions["dvEnabled"].toBool();
      qDebug() << "DV playback enabled:" << playbackConfig.dvEnabled;
    }
  }
}
void CameraConfig::setClassificationModelFilePath(const QString &model) {
  qDebug() << "model file path:" << model;
  classificationModelFilePath = model;
}
void CameraConfig::setClassificationModel(const QString &model) {
  classificationModel = model;
  qDebug() << "classification model:" << model;
}
void CameraConfig::setClassificationModelPolicy(const QString &policy) {
  classificationModelPolicy = policy;
  qDebug() << "classification policy:" << policy;
}