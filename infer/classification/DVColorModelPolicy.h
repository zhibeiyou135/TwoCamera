#pragma once
#include "ClassificationModelPolicy.h"
#include "NiguangModelPolicy.h"
class DVColorModelPolicy : public NiguangModelPolicy {
public:
  int getInputSize() override;
  int getOutputSize() override { return sizeof(float) * CLASSES; }
  uchar *processInputImg(const cv::Mat &img) override;
  // const QString &processOutput(uchar *out) override;
  ~DVColorModelPolicy() {}
};