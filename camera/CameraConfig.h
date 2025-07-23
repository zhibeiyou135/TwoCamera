//
// Created by pe on 2020/10/12.
//

#ifndef TWOCAMERA_CAMERACONFIG_H
#define TWOCAMERA_CAMERACONFIG_H

#include <QString>
#include <QVector>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <tuple>
struct CameraControlConfig {
  //是否有回放模式
  bool playback = false;
  //是否开启分类功能
  bool classification = false;
  bool denoise = false;
  bool speed = false;
  //是否开启dv
  bool dv = false;
  bool overlap = false;
  //控制保存图片的格式
  bool saveFormat = false;
  //选择深度学习模型
  bool modelSelect = false;
  //选择深度学习模型对应的策略
  bool modelPolicySelect = false;
  //保存选项，控制保存那些内容
  bool saveOptions = false;
  //重构，控制是否开启重构功能
  bool construct = false;
  bool threshold = false;
};

// 回放配置结构体
struct PlaybackConfig {
  QString mode = "both"; // "dv", "dvs", or "both"
  bool dvsEnabled = true;
  bool dvEnabled = true;
  bool enabled = true;    // 是否启用回放功能
};

class CameraConfig {
private:
  int resultMapPolicy = 0;
  bool debugMode = false;
  CameraControlConfig cameraControlConfig;
  PlaybackConfig playbackConfig;
  QString classificationModelFilePath, classificationModelPolicy,
      classificationModel;
  int classificationThreads = 1;
  int fontPointSize = 12;
  bool dvsViewVerticalFlip = false, dvViewVerticalFlip = false,
       dvViewHorizontalFlip = false, dvsViewHorizontalFlip = false;

public:
  static CameraConfig &getInstance() {
    static CameraConfig cc;
    return cc;
  }
  //从配置文件中获取配置信息
  void loadConfigFile(const QString &file = "config.json");

  void setResultMapPolicy(int c) { resultMapPolicy = c; }

  int getResultMapPolicy() const { return resultMapPolicy; }
  int getFontPointSize() { return fontPointSize; }

  void setDebugMode(bool d) { debugMode = d; }

  bool getDebugMode() { return debugMode; }
  void setClassificationModelPolicy(const QString &policy);
  void setClassificationModel(const QString &model);
  void setClassificationModelFilePath(const QString &model);
  bool getDVSViewVerticalFlip() { return dvsViewVerticalFlip; }
  bool getDVSViewHorizontalFlip() { return dvsViewHorizontalFlip; }
  bool getDVViewVerticalFlip() { return dvViewVerticalFlip; }
  bool getDVViewHorizontalFlip() { return dvViewHorizontalFlip; }
  std::string dvImageSaveFormat;
  CameraControlConfig &getControlConfig() { return cameraControlConfig; }
  
  // 获取回放配置
  PlaybackConfig &getPlaybackConfig() { return playbackConfig; }
};

#endif // TWOCAMERA_CAMERACONFIG_H
