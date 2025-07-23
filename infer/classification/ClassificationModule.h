#ifndef CLASSIFICATION_MODULE_H
#define CLASSIFICATION_MODULE_H
#include "BlockingQueue.h"
#include "ClassificationModelBase.h"
#include "ClassificationModelPolicy.h"
#include <QObject>
#include <QString>
#include <boost/lockfree/queue.hpp>
#include <functional>
#include <memory>
#include <opencv2/opencv.hpp>
class ClassificationModule : public QObject {
  Q_OBJECT
private:
  typedef ClassificationModelPolicy *(
      *ClassificationModelPolicyFactoryFunction)();
  BlockingQueue<std::shared_ptr<cv::Mat>> imgs;
  QString modelPath;
  std::function<ClassificationModelPolicy *()> modelPolicyFactory;
  std::function<ClassificationModelBase *(const QString &)> modelFactory;
  
  // 私有构造函数，确保单例
  ClassificationModule();
  
  // 静态实例
  static ClassificationModule* instance;

public:
  // 获取单例实例
  static ClassificationModule* getInstance() {
    if (!instance) {
      instance = new ClassificationModule();
    }
    return instance;
  }

  void setModelPath(const QString &file) { modelPath = file; }
  void enqueue(std::shared_ptr<cv::Mat> img);
  void setModelPolicyFactory(decltype(modelPolicyFactory) f);
  void setModelFactory(decltype(modelFactory) f);
  void startInfer(int threads = 1);
signals:
  void result(QString label);
};
#endif