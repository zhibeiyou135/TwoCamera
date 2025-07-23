#include "./DetectModule.h"
#include "./TorchEngine.h"
#include <QImage>
#include <QtConcurrent>
#include <thread>
#include <vector>
using namespace cv;
void DetectModule::startInfer(int threads) {
  for (int i = 0; i < threads; i++) {
    QtConcurrent::run([this]() {
      TorchEngine e(this->modelPath);
      while (true) {
        // 使用非阻塞的tryPop避免无限等待导致UI卡死
        std::shared_ptr<cv::Mat> img;
        if (!imgQueue.tryPop(img, 500)) {
          // 如果500ms内没有新图像，继续循环
          continue;
        }
        
        //这里改成单通道的了
        auto tensor = new cv::Mat(img->clone());
        //auto channel = img->clone();
        //cv::merge(std::vector<cv::Mat>{channel}, *tensor);
        Labels = "识别结果:";
        tensor = e.infer(tensor,Labels);

        //cv::merge(std::vector<cv::Mat>{tensor,tensor,tensor}, *tensor);
        emit this->newDetectResultImg(QImage(
            tensor->data, tensor->cols, tensor->rows, tensor->step,
            QImage::Format_Grayscale8, [](void *p) { delete (cv::Mat *)p; },
            tensor));
        emit this->newResult(Labels);
      }
    });
  }
}
void DetectModule::enqueue(std::shared_ptr<cv::Mat> img) {
  imgQueue.clear();
  imgQueue.push(img);
}
