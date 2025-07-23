#pragma once
#include "BlockingQueue.h"
#include <QObject>
#include <QImage>
#include <memory>
#include <opencv2/opencv.hpp>

class DetectModule : public QObject {
  Q_OBJECT
private:
  DetectModule() {}
  QString modelPath;
  BlockingQueue<std::shared_ptr<cv::Mat>> imgQueue;
  QString Labels;
public:
  static DetectModule *getInstance() {
    static DetectModule instance;
    return &instance;
  }
  void setModelPath(const QString &path) { modelPath = path;}
  void enqueue(std::shared_ptr<cv::Mat> img);
  void startInfer(int threads = 1);

signals:
  void newDetectResultImg(QImage);
  void newResult(QString);
};
