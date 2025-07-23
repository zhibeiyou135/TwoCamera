#include "SingleDVSConstructModelPolicy.h"
using namespace construct;
void *SingleDVSConstructModelPolicy::processInput(cv::Mat &img) {
  float *data = new float[IMG_WIDTH * IMG_HEIGHT];
  cv::Mat out(IMG_HEIGHT, IMG_WIDTH, CV_32FC1, data);
  img.convertTo(out, CV_32FC1, 1.0 / 255.0);
  return data;
}
QImage SingleDVSConstructModelPolicy::processOutput(void *data) {
  cv::Mat img(IMG_HEIGHT, IMG_WIDTH, CV_32FC1, data);
  cv::Mat *out = new cv::Mat();
  img.convertTo(*out, CV_8UC1, 255.0);
  return QImage(
      out->data, out->cols, out->rows, out->step, QImage::Format_Grayscale8,
      [](void *p) { delete (cv::Mat *)p; }, out);
}