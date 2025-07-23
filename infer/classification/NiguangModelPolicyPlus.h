#pragma once
#include "infer/classification/NiguangModelPolicy.h"
class NiguangModelPolicyPlus : public NiguangModelPolicy {
private:
  static const int QUEUE_SIZE = 60;
  int queue[QUEUE_SIZE];
  int queueIndex = 0;
  int labelIndexCount[CLASSES];

public:
  NiguangModelPolicyPlus() {
    memset(queue, 0, sizeof(queue));
    memset(labelIndexCount, 0, sizeof(labelIndexCount));
  }
  // int getInputSize() override;
  // int getOutputSize() override { return sizeof(float) * CLASSES; }
  // uchar *processInputImg(const cv::Mat &img) override;
  const QString &processOutput(uchar *out);
  ~NiguangModelPolicyPlus() {}
};