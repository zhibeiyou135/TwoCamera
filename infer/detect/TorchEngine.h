#pragma once

#ifdef TORCH_ENABLE
#undef slots
// 添加必要的标准库头文件
#include <memory>
#include <optional>
// 只包含必要的torch头文件，避免Python依赖
#include <torch/script.h>
#include <torch/nn.h>
#include <torch/data.h>
#define slots Q_SLOTS
#endif

#include <QString>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

const int HUGE_STEP = 4096;
const float CONF_THRE = 0.45, IOU_THRE = 0.2;
const float WIDTH_SCALE = 1280.0 / 640.0;
const float HEIGHT_SCALE = 800.0 / 416.0;

class TorchEngine {
private:
  const int color_list[10][3] = {
      {0, 255, 0},   {255, 0, 0},   {0, 0, 255},
      {255, 255, 0}, {0, 255, 255},
      {127,255,255},{255,127,255},{255,255,127},{127,127,255},{127,255,127}
  };
  struct Box {
    int left;
    int top;
    int right;
    int bottom;
    int cls;
    float confidence;
  };
  const std::string name[10]{"starfish", "seahorse", "crab",
                            "turtle",    "whale","car1","car2","car3","tank","helicopter"};
  static const int WIDTH = 640;
  static const int HEIGHT = 416;
  static const int CHANNEL = 1;
  //static const int OUTMAPNUM=3900;
  //static const int CLASSNUM=5;
  static const int OUTMAPNUM=2600;
  static const int CLASSNUM=10;
#ifdef TORCH_ENABLE
  torch::jit::script::Module model;
#endif
  cv::Mat *processInput(cv::Mat *input);
  cv::Mat *processOutput(cv::Mat *img, cv::Mat *output, QString & Labels);
  void NMS(std::vector<Box> &boxes);           //非极大抑制
  float IOU(const Box &box1, const Box &box2); //算IOU
  void paintbox(cv::Mat &img, const std::vector<Box> &boxes);
  static bool CmpConf(const Box &b1, const Box &b2) {
    return b1.confidence > b2.confidence;
  }

public:
  TorchEngine(const QString &model);
  cv::Mat *infer(cv::Mat *tensor,QString & Labels);
};
