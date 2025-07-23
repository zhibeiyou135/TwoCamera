#ifndef INFER_SINGLE_DVS_CONSTRUCT_MODEL_POLICY_H
#define INFER_SINGLE_DVS_CONSTRUCT_MODEL_POLICY_H
#include "SingleDVSConstructModelPolicyBase.h"
namespace construct {
class SingleDVSConstructModelPolicy : public SingleDVSConstructModelPolicyBase {
private:
  static const int IMG_WIDTH = 1280, IMG_HEIGHT = 720;

public:
  int getInputSize() { return IMG_HEIGHT * IMG_WIDTH * sizeof(float); }
  int getOutputSize() { return IMG_WIDTH * IMG_HEIGHT * sizeof(float); }
  void *processInput(cv::Mat &img);
  QImage processOutput(void *);
};
} // namespace construct
#endif