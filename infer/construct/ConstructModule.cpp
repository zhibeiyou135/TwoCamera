#include "ConstructModule.h"
#include <QtConcurrent>
using namespace construct;
void ConstructModule::startInfer(int threads) {
  for (int i = 0; i < threads; i++)
    QtConcurrent::run([this]() {
      auto policy = modelPolicyFactory();
      auto model = modelFactory(modelPath, policy);
      for (;;) {
        std::shared_ptr<cv::Mat> img;
        if (!imgQueue.tryPop(img, 500)) {
          continue;
        }
        
        auto modelInput = policy->processInput(*img);
        auto modelOutput = model->infer(modelInput);
        auto res = policy->processOutput(modelOutput);
        delete[](uchar *) modelInput;
        delete[](uchar *) modelOutput;
        emit newDVSConstructImg(res);
      }
    });
}
void ConstructModule::enqueue(std::shared_ptr<cv::Mat> img) {
  imgQueue.clear();
  imgQueue.push(img);
}