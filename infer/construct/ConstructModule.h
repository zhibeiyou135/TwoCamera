#pragma once
#include "BlockingQueue.h"
#include "SingleDVSConstructModel.h"
#include "SingleDVSConstructModelPolicyBase.h"
#include <QObject>
#include <QString>
#include <functional>
#include <memory>
#include <opencv2/opencv.hpp>
namespace construct {
class ConstructModule : public QObject {
  Q_OBJECT
private:
  std::function<SingleDVSConstructModelPolicyBase *()> modelPolicyFactory;
  std::function<SingleDVSConstructModel *(
      const QString &path, SingleDVSConstructModelPolicyBase *policy)>
      modelFactory;
  QString modelPath;
  BlockingQueue<std::shared_ptr<cv::Mat>> imgQueue;
  ConstructModule(){};

public:
  static ConstructModule *getInstance() {
    static ConstructModule instance;
    return &instance;
  }
  void startInfer(int threads = 1);
  void enqueue(std::shared_ptr<cv::Mat> img);
  void setModelPolicyFactory(decltype(modelPolicyFactory) factory) {
    modelPolicyFactory = factory;
  }
  void setModelFactory(decltype(modelFactory) factory) {
    modelFactory = factory;
  }
  void setModelPath(const QString &path) { modelPath = path; }
signals:
  void newDVSConstructImg(QImage);
};
} // namespace construct