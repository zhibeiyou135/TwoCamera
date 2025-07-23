#include "ClassificationModule.h"
#include "ClassificationModelBase.h"
#include "TRTModel.h"
#include <QtConcurrent>
#include <thread>

// 初始化静态成员变量
ClassificationModule* ClassificationModule::instance = nullptr;

ClassificationModule::ClassificationModule() : imgs(1) {}

void ClassificationModule::enqueue(std::shared_ptr<cv::Mat> img) {
  if (modelPath == "") {
    //没有载入模型退出
    return;
  }
  imgs.push(img);
}

void ClassificationModule::setModelPolicyFactory(
    decltype(modelPolicyFactory) f) {
  modelPolicyFactory = f;
}

void ClassificationModule::setModelFactory(decltype(modelFactory) f) {
  modelFactory = f;
}
void ClassificationModule::startInfer(int threads) {
  using namespace std;
  qDebug() << "classification start infer " << threads << " threads";
  for (int i = 0; i < threads; i++) {
    auto policy = modelPolicyFactory();
    auto model = modelFactory(modelPath);
    model->setModelPolicy(policy);
    model->init();
    QtConcurrent::run([this, policy, model]() {
      stringstream ss;
      ss << this_thread::get_id();
      auto threadId = ss.str().c_str();
      qDebug() << "start classification thread " << threadId;
      for (;;) {
        // 使用非阻塞的tryPop避免无限等待导致UI卡死
        std::shared_ptr<cv::Mat> img;
        if (!imgs.tryPop(img, 500)) {
          // 如果500ms内没有新图像，继续循环
          continue;
        }
        
        auto modelInput = policy->processInputImg(*img);
        auto modelOutput = model->infer(modelInput);
        auto label = policy->processOutput(modelOutput);
        delete[] modelOutput;
        delete[] modelInput;
        emit this->result(label);
        qDebug() << "class result label:" << label << " in thread " << threadId;
      }
    });
  }
}
