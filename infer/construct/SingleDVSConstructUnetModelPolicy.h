#ifndef INFER_CONSTRUCT_SINGLE_DVS_CONSTRUCT_UNET_MODEL_POLICY_H
#define INFER_CONSTRUCT_SINGLE_DVS_CONSTRUCT_UNET_MODEL_POLICY_H
#include "SingleDVSConstructModelPolicyBase.h"
namespace construct {
class SingleDVSConstructUnetModelPolicy
    : public SingleDVSConstructModelPolicyBase {
private:
  static const int IMG_WIDTH = 1280, IMG_HEIGHT = 800;

public:
  int getInputSize() { return IMG_HEIGHT * IMG_WIDTH * sizeof(float) * 3; }
  int getOutputSize() { return IMG_WIDTH * IMG_HEIGHT * sizeof(float) * 3; }
  void *processInput(cv::Mat &img);
  QImage processOutput(void *);
};
} // namespace construct
#endif