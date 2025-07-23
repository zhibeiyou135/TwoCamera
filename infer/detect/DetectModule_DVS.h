#pragma once
#include "BlockingQueue.h"
#include <QObject>
#include <QImage>
#include <memory>
#include <opencv2/opencv.hpp>

// 前向声明
class DVSDetectWorker;
class TorchEngine_DVS;

class DetectModule_DVS : public QObject {
  Q_OBJECT
  
  // 声明工作线程为友元类
  friend class DVSDetectWorker;
  
private:
  DetectModule_DVS();
  ~DetectModule_DVS();
  QString modelPath;
  BlockingQueue<std::shared_ptr<cv::Mat>> imgQueue;
  QString Labels;
  DVSDetectWorker* worker; // 工作线程指针
  std::unique_ptr<TorchEngine_DVS> engine; // 模型引擎，在初始化时加载
  bool modelLoaded; // 模型是否已加载
  bool inferenceRunning; // 推理线程是否正在运行
  
public:
  static DetectModule_DVS *getInstance() {
    static DetectModule_DVS instance;
    return &instance;
  }
  
  // 设置模型路径并立即加载模型
  bool setModelPath(const QString &path);
  
  // 启动/停止推理线程（不重新加载模型）
  void startInference();
  void stopInference();
  
  // 检查模型是否已加载
  bool isModelLoaded() const { return modelLoaded; }
  
  // 检查推理是否正在运行
  bool isInferenceRunning() const { return inferenceRunning; }
  
  // 图像入队
  void enqueue(std::shared_ptr<cv::Mat> img);
  
  // 获取模型引擎（线程安全）
  TorchEngine_DVS* getEngine() { return engine.get(); }

signals:
  void newDetectResultImg(QImage);
  void newResult(QString);
  void modelLoadResult(bool success, QString message);
}; 