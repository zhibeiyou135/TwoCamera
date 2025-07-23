#include "SingleDVSConstructUnetModelPolicy.h"
using namespace construct;
void *SingleDVSConstructUnetModelPolicy::processInput(cv::Mat &img) {
  cv::Mat resize;
  cv::resize(img, resize, cv::Size(IMG_WIDTH, IMG_HEIGHT));
  uchar *data = new uchar[IMG_WIDTH * IMG_HEIGHT * 3 * sizeof(float)];
  cv::Mat out(IMG_HEIGHT, IMG_WIDTH, CV_32FC1, data);
  resize.convertTo(out, CV_32FC1, 1.0 / 255.0);
  memcpy(data + (IMG_HEIGHT * IMG_WIDTH * sizeof(float)), data,
         (IMG_HEIGHT * IMG_WIDTH * sizeof(float)));
  memcpy(data + (IMG_HEIGHT * IMG_WIDTH * sizeof(float) * 2), data,
         (IMG_HEIGHT * IMG_WIDTH * sizeof(float)));
  return data;
}
QImage SingleDVSConstructUnetModelPolicy::processOutput(void *data) {
  cv::Mat img(IMG_HEIGHT, IMG_WIDTH, CV_32FC1, data);
  cv::Mat *out = new cv::Mat();
  img.convertTo(*out, CV_8UC1, 255.0);
  return QImage(
      out->data, out->cols, out->rows, out->step, QImage::Format_Grayscale8,
      [](void *p) { delete (cv::Mat *)p; }, out);
}