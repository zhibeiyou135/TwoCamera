#include "DVColorModelPolicy.h"
int DVColorModelPolicy::getInputSize() {
  return IMG_HEIGHT * IMG_WIDTH * 3 * sizeof(float);
}
uchar *DVColorModelPolicy::processInputImg(const cv::Mat &img) {
  uchar *data = new uchar[getInputSize()];
  cv::Mat out;
  cv::Mat res(IMG_WIDTH, IMG_HEIGHT, CV_32FC3, data);
  img.convertTo(out, CV_32FC3, 1.0 / 255.0);
  cv::resize(out, res, cv::Size(IMG_HEIGHT, IMG_WIDTH));
  // cv::subtract(res, cv::Scalar(1), res);
  memcpy(data, res.data, getInputSize());
  return data;
}