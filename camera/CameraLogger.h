//
// Created by pe on 2021/3/8.
//

#ifndef TWOCAMERA_CAMERALOGGER_H
#define TWOCAMERA_CAMERALOGGER_H

#include "CameraConfig.h"
#include <QDebug>
#include <QDir>
#include <QTemporaryFile>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <string>

class CameraLogger {
private:
  std::string p;
  //    std::string imgFormat = CameraConfig::getInstance().dvImageSaveFormat;
public:
  CameraLogger(const std::string &path) { p = path; }

  void stopRecording() {}

  void saveToPath(const QString &path) {}

  void addImage(cv::Mat *img) {
    cv::imwrite(p+"/" +std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count()) +
                    ".png",
                *img);
  }

  ~CameraLogger() { stopRecording(); }
};

#endif // TWOCAMERA_CAMERALOGGER_H
