//
// Created by pe on 2021/7/8.
//

#include "infer/classification/EightModelPolicy.h"
int EightModelPolicy::getInputSize() {
  return IMG_WIDTH * IMG_HEIGHT * sizeof(float);
}
uchar *EightModelPolicy::processInputImg(const cv::Mat &img) {
  uchar *data = new uchar[IMG_WIDTH * IMG_HEIGHT * sizeof(float)];
  cv::Mat out;
  cv::Mat res(IMG_WIDTH, IMG_HEIGHT, CV_32FC1, data);
  img.convertTo(out, CV_32FC1, 1.0 / 127.5);
  cv::resize(out, res, cv::Size(IMG_HEIGHT, IMG_WIDTH));
  cv::subtract(res, cv::Scalar(1), out);
  memcpy(data, out.data, getInputSize());
  return data;
}
const QString &EightModelPolicy::processOutput(uchar *out) {
  float *data = (float *)out;
  return ls[argmax(data, CLASSES)];
}