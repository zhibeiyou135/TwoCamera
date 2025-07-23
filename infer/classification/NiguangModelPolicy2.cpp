#include "NiguangModelPolicy2.h"
int NiguangModelPolicy2::getInputSize() {
  return IMG_WIDTH * IMG_HEIGHT * sizeof(float) * OVERLAP;
}
uchar *NiguangModelPolicy2::processInputImg(const cv::Mat &img) {
  uchar *data = new uchar[IMG_WIDTH * IMG_HEIGHT * sizeof(float) * OVERLAP];
  cv::Mat out;
  cv::Mat corp=img(cv::Range(112,688),cv::Range(352,928));
  cv::Mat cvt_out=img(cv::Range(112,688),cv::Range(352,928));
  corp.convertTo(cvt_out, CV_32FC1, 1.0 / 255.0);
  imgs.erase(imgs.begin());
  imgs.push_back(cvt_out);
  cv::merge(imgs,out);
  memcpy(data, out.data, getInputSize());
  return data;
}
const QString &NiguangModelPolicy2::processOutput(uchar *out) {
  float *data = (float *)out;
  return ls[argmax(data, CLASSES)];
}
