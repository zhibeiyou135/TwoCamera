#include "DVNiguangModelPolicy.h"

uchar *DVNiguangModelPolicy::processInputImg(const cv::Mat &img) {
  uchar *data = new uchar[IMG_WIDTH * IMG_HEIGHT * sizeof(float)];
  cv::Mat out;
  cv::Mat res(IMG_WIDTH, IMG_HEIGHT, CV_32FC1, data);
  cv::cvtColor(img, out, cv::COLOR_RGB2GRAY);
  out.convertTo(out, CV_32FC1, 1.0 / 255.0);
  cv::resize(out, res, cv::Size(IMG_HEIGHT, IMG_WIDTH));
  // cv::subtract(res, cv::Scalar(1), res);
  memcpy(data, res.data, getInputSize());
  return data;
}