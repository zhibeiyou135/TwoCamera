//
// Created by pe on 2021/9/16.
//

#include "CamerasDetector.h"
#include <QDebug>
#include <opencv2/opencv.hpp>

CamerasDetector::CamerasDetector() { detectCameras(); }

void CamerasDetector::detectCameras() {
  camerasIndex.clear();
  int items = 0;
  for (int i = 0; i < 10; i++) {
    cv::VideoCapture vc;
    vc.open(i, cv::CAP_V4L2);
    if (vc.isOpened()) {
      // qDebug() << i;
      camerasIndex.push_back(i);
      items++;
      vc.release();
    }
  }
}
