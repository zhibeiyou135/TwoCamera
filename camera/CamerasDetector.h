//
// Created by pe on 2021/9/16.
//

#ifndef TWOCAMERA_CAMERASDETECTOR_H
#define TWOCAMERA_CAMERASDETECTOR_H

#include <vector>

class CamerasDetector {
private:
  CamerasDetector();

  std::vector<int> camerasIndex;

public:
  static CamerasDetector *getInstance() {
    static CamerasDetector instance;
    return &instance;
  }

  void detectCameras();

  const std::vector<int> &getCamerasIndex() { return camerasIndex; }
};

#endif // TWOCAMERA_CAMERASDETECTOR_H
