#pragma once
#include "NiguangModelPolicy.h"
class DVNiguangModelPolicy : public NiguangModelPolicy {

public:
  uchar *processInputImg(const cv::Mat &img) override;
};