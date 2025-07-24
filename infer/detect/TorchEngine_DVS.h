#pragma once
//#ifdef TORCH_ENABLE
#undef slots
#include <memory>
#include <optional>
#include <torch/script.h>
#include <torch/nn.h>
#include <torch/data.h>
#define slots Q_SLOTS
//#endif
#include <QString>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

const int DVS_HUGE_STEP = 4096;
const float DVS_CONF_THRE = 0.1, DVS_IOU_THRE = 0.02;
const float DVS_WIDTH_SCALE = 1280.0 / 640.0;
const float DVS_HEIGHT_SCALE = 720.0 / 416.0;

class TorchEngine_DVS {
private:
  const int color_list[4][3] = {
      {0, 255, 0},    // 绿色 - 缺陷类别1
      {255, 0, 0},    // 红色 - 缺陷类别2
      {0, 0, 255},    // 蓝色 - 缺陷类别3
      {255, 255, 0}   // 黄色 - 缺陷类别4
  };

  struct Box {
    int left;
    int top;
    int right;
    int bottom;
    int cls;
    float confidence;
  };

  const std::string name[4]{"huanhen", "dian", "wuzi", "liangdai"};
  static const int WIDTH = 640;
  static const int HEIGHT = 416;
  static const int CHANNEL = 3;
  static const int OUTMAPNUM=16380;
  static const int CLASSNUM=4;
  
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
  TorchEngine_DVS(const QString &model);
  cv::Mat *infer(cv::Mat *tensor, QString & Labels);
}; 