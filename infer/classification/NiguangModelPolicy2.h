#pragma once
#include "ClassificationModelPolicy.h"
#include <QStringList>
#include <vector>
class NiguangModelPolicy2 : public ClassificationModelPolicy {
protected:
  // static const int CLASSES = 5;
  static const int CLASSES = 5;
  static const int IMG_HEIGHT = 576, IMG_WIDTH = 576;
  static const int OVERLAP = 10;
  std::vector<cv::Mat> imgs;
  // QString ls[CLASSES]{"飞机", "火箭", "卫星1", "卫星2", "未知"};
  //QString ls[CLASSES]{"无目标", "卫星一", "卫星二", "卫星三", "未知"};
  QString ls[CLASSES]{"crab","seafish", "seahorse", "turtle", "whale"};
  

public:
  NiguangModelPolicy2() {imgs=std::vector<cv::Mat>(OVERLAP,cv::Mat::zeros(IMG_HEIGHT,IMG_WIDTH,CV_32FC1));}
  int getInputSize() override;
  int getOutputSize() override { return sizeof(float) * CLASSES; }
  uchar *processInputImg(const cv::Mat &img) override;
  const QString &processOutput(uchar *out) override;
  ~NiguangModelPolicy2() {}
};
