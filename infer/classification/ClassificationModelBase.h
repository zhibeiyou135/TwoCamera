#ifndef CLASSIFICATION_MODEL_BASE_H
#define CLASSIFICATION_MODEL_BASE_H
#include "ClassificationModelPolicy.h"
#include <QByteArray>
#include <QString>
#include <opencv2/opencv.hpp>
#include <vector>
class ClassificationModelBase {
private:
  static ClassificationModelBase *(*factory)(const QString &modelPath);

protected:
  static QByteArray readNet(const QString &path);
  ClassificationModelPolicy *policy = nullptr;

public:
  static ClassificationModelBase *newModel(const QString &modelPath) {
    return factory(modelPath);
  }
  static void
  setModelFactory(ClassificationModelBase *(*f)(const QString &modelPath)) {
    factory = f;
  }
  virtual void setModelPolicy(ClassificationModelPolicy *p) {
    delete policy;
    policy = p;
  }
  virtual void init() = 0;
  virtual uchar *infer(const uchar *data) = 0;
  virtual ~ClassificationModelBase() { delete policy; }
};
#endif