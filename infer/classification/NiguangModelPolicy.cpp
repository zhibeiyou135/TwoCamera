#include "NiguangModelPolicy.h"
int NiguangModelPolicy::getInputSize() {
  return IMG_WIDTH * IMG_HEIGHT * sizeof(float);
}
uchar *NiguangModelPolicy::processInputImg(const cv::Mat &img) {
  uchar *data = new uchar[IMG_WIDTH * IMG_HEIGHT * sizeof(float)];
  cv::Mat out;
  cv::Mat res(IMG_WIDTH, IMG_HEIGHT, CV_32FC1, data);
  img.convertTo(out, CV_32FC1, 1.0 / 255.0);
  cv::resize(out, res, cv::Size(IMG_HEIGHT, IMG_WIDTH));
  // cv::subtract(res, cv::Scalar(1), res);
  memcpy(data, res.data, getInputSize());
  return data;
}
const QString &NiguangModelPolicy::processOutput(uchar *out) {
  float *data = (float *)out;
  return ls[argmax(data, CLASSES)];
}