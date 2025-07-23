#ifndef CLASSIFICATION_MODEL_POLICY_H
#define CLASSIFICATION_MODEL_POLICY_H
#include <QString>
#include <opencv2/opencv.hpp>
class ClassificationModelPolicy {
private:
  static ClassificationModelPolicy *instance;

protected:
  ClassificationModelPolicy() {}

public:
  static ClassificationModelPolicy *getInstance() { return instance; }
  static void setInstance(ClassificationModelPolicy *ins) {
    delete ClassificationModelPolicy::instance;
    instance = ins;
  }
  virtual int getInputSize() = 0;
  virtual int getOutputSize() = 0;
  virtual uchar *processInputImg(const cv::Mat &img) = 0;
  virtual const QString &processOutput(uchar *out) = 0;
  template <typename T> static int argmax(T *data, int size) {
    auto m = data[0];
    int k = 0;
    for (int i = 1; i < size; i++) {
      if (data[i] > m) {
        m = data[i];
        k = i;
      }
    }
    return k;
  }
  virtual ~ClassificationModelPolicy() {}
};
#endif