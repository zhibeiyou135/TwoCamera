#pragma once
#include "ClassificationModelPolicy.h"
#include <QStringList>

class NiguangModelPolicy : public ClassificationModelPolicy {
protected:
  // static const int CLASSES = 5;
  static const int CLASSES = 6;
  static const int IMG_HEIGHT = 224, IMG_WIDTH = 224;
  // QString ls[CLASSES]{"飞机", "火箭", "卫星1", "卫星2", "未知"};
  //QString ls[CLASSES]{"无目标", "卫星一", "卫星二", "卫星三", "未知"};
  QString ls[CLASSES]{"宇航员","汽车", "无目标", "火箭", "卫星", "未知"};
  

public:
  NiguangModelPolicy() {}
  int getInputSize() override;
  int getOutputSize() override { return sizeof(float) * CLASSES; }
  uchar *processInputImg(const cv::Mat &img) override;
  const QString &processOutput(uchar *out) override;
  ~NiguangModelPolicy() {}
};
