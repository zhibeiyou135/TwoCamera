//
// Created by pe on 2021/7/8.
//

#ifndef TWOCAMERA_EIGHTMODELPOLICY_H
#define TWOCAMERA_EIGHTMODELPOLICY_H

#include "ClassificationModelPolicy.h"
#include <QStringList>

class EightModelPolicy : public ClassificationModelPolicy {
private:
  static const int CLASSES = 9, IMG_HEIGHT = 224, IMG_WIDTH = 224;
  QString ls[CLASSES]{"导弹", "航母",   "轰炸机", "火炮",  "未知",
                      "船",   "运输机", "直升机", "装甲车"};

public:
  EightModelPolicy() {}
  int getInputSize() override;
  int getOutputSize() override { return sizeof(float) * CLASSES; }
  uchar *processInputImg(const cv::Mat &img) override;
  const QString &processOutput(uchar *out);
  ~EightModelPolicy() {}
};

#endif // TWOCAMERA_EIGHTMODELPOLICY_H
