#ifndef INFER_SINGLE_DVS_CONSTRUCT_MODEL_POLICY_BASE_H
#define INFER_SINGLE_DVS_CONSTRUCT_MODEL_POLICY_BASE_H
#include <QImage>
#include <opencv2/opencv.hpp>
namespace construct {
class SingleDVSConstructModelPolicyBase {
public:
  virtual int getInputSize() = 0;
  virtual int getOutputSize() = 0;
  virtual void *processInput(cv::Mat &img) = 0;
  virtual QImage processOutput(void *) = 0;
  virtual ~SingleDVSConstructModelPolicyBase() {}
};
} // namespace construct
#endif
