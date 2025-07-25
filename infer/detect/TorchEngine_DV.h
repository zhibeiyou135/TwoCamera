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
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDebug>
#include "../camera/ConfigManager.h"

// 检测配置结构体
struct DVDetectionConfig {
    float confidence_threshold = 0.45f;
    float iou_threshold = 0.2f;
    float min_area = 100.0f;           // 最小检测区域面积
    float max_area = 50000.0f;         // 最大检测区域面积
    float min_aspect_ratio = 0.2f;     // 最小宽高比
    float max_aspect_ratio = 5.0f;     // 最大宽高比
    bool enable_area_filter = true;    // 启用面积过滤
    bool enable_aspect_filter = true;  // 启用宽高比过滤

    // 高级过滤参数
    float min_width = 10.0f;           // 最小宽度
    float min_height = 10.0f;          // 最小高度
    float max_width = 1000.0f;         // 最大宽度
    float max_height = 1000.0f;        // 最大高度
    bool enable_size_filter = true;    // 启用尺寸过滤

    // 置信度分层过滤
    float high_conf_threshold = 0.8f;  // 高置信度阈值
    float medium_conf_threshold = 0.6f; // 中等置信度阈值
    bool enable_confidence_layers = false; // 启用置信度分层

    // 边界过滤
    float border_margin = 5.0f;        // 边界边距
    bool enable_border_filter = false; // 启用边界过滤

    // 从JSON加载配置
    void loadFromJson(const QJsonObject& configObj, const QString& cameraType) {
        if (configObj.contains("confidence_threshold") && configObj["confidence_threshold"].isDouble()) {
            confidence_threshold = static_cast<float>(configObj["confidence_threshold"].toDouble());
        }
        if (configObj.contains("iou_threshold") && configObj["iou_threshold"].isDouble()) {
            iou_threshold = static_cast<float>(configObj["iou_threshold"].toDouble());
        }
        if (configObj.contains("min_area") && configObj["min_area"].isDouble()) {
            min_area = static_cast<float>(configObj["min_area"].toDouble());
        }
        if (configObj.contains("max_area") && configObj["max_area"].isDouble()) {
            max_area = static_cast<float>(configObj["max_area"].toDouble());
        }
        if (configObj.contains("min_aspect_ratio") && configObj["min_aspect_ratio"].isDouble()) {
            min_aspect_ratio = static_cast<float>(configObj["min_aspect_ratio"].toDouble());
        }
        if (configObj.contains("max_aspect_ratio") && configObj["max_aspect_ratio"].isDouble()) {
            max_aspect_ratio = static_cast<float>(configObj["max_aspect_ratio"].toDouble());
        }
        if (configObj.contains("enable_area_filter") && configObj["enable_area_filter"].isBool()) {
            enable_area_filter = configObj["enable_area_filter"].toBool();
        }
        if (configObj.contains("enable_aspect_filter") && configObj["enable_aspect_filter"].isBool()) {
            enable_aspect_filter = configObj["enable_aspect_filter"].toBool();
        }

        // 高级过滤参数
        if (configObj.contains("min_width") && configObj["min_width"].isDouble()) {
            min_width = static_cast<float>(configObj["min_width"].toDouble());
        }
        if (configObj.contains("min_height") && configObj["min_height"].isDouble()) {
            min_height = static_cast<float>(configObj["min_height"].toDouble());
        }
        if (configObj.contains("max_width") && configObj["max_width"].isDouble()) {
            max_width = static_cast<float>(configObj["max_width"].toDouble());
        }
        if (configObj.contains("max_height") && configObj["max_height"].isDouble()) {
            max_height = static_cast<float>(configObj["max_height"].toDouble());
        }
        if (configObj.contains("enable_size_filter") && configObj["enable_size_filter"].isBool()) {
            enable_size_filter = configObj["enable_size_filter"].toBool();
        }
        if (configObj.contains("high_conf_threshold") && configObj["high_conf_threshold"].isDouble()) {
            high_conf_threshold = static_cast<float>(configObj["high_conf_threshold"].toDouble());
        }
        if (configObj.contains("medium_conf_threshold") && configObj["medium_conf_threshold"].isDouble()) {
            medium_conf_threshold = static_cast<float>(configObj["medium_conf_threshold"].toDouble());
        }
        if (configObj.contains("enable_confidence_layers") && configObj["enable_confidence_layers"].isBool()) {
            enable_confidence_layers = configObj["enable_confidence_layers"].toBool();
        }
        if (configObj.contains("border_margin") && configObj["border_margin"].isDouble()) {
            border_margin = static_cast<float>(configObj["border_margin"].toDouble());
        }
        if (configObj.contains("enable_border_filter") && configObj["enable_border_filter"].isBool()) {
            enable_border_filter = configObj["enable_border_filter"].toBool();
        }

        qDebug() << cameraType << "检测配置加载完成:";
        qDebug() << "  confidence_threshold:" << confidence_threshold;
        qDebug() << "  iou_threshold:" << iou_threshold;
        qDebug() << "  min_area:" << min_area << "max_area:" << max_area;
        qDebug() << "  min_aspect_ratio:" << min_aspect_ratio << "max_aspect_ratio:" << max_aspect_ratio;
        qDebug() << "  enable_area_filter:" << enable_area_filter;
        qDebug() << "  enable_aspect_filter:" << enable_aspect_filter;
        qDebug() << "  enable_size_filter:" << enable_size_filter;
        qDebug() << "  enable_confidence_layers:" << enable_confidence_layers;
        qDebug() << "  enable_border_filter:" << enable_border_filter;
    }
};

const int DV_HUGE_STEP = 4096;
const float DV_WIDTH_SCALE = 1280.0 / 640.0;
const float DV_HEIGHT_SCALE = 720.0 / 416.0;

class TorchEngine_DV {
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
  //static const int OUTMAPNUM=3600;
  //static const int CLASSNUM=5;
  static const int OUTMAPNUM=16380;
  static const int CLASSNUM=4;

  // 检测配置
  DVDetectionConfig detectionConfig;

#ifdef TORCH_ENABLE
  torch::jit::script::Module model;
#endif
  cv::Mat *processInput(cv::Mat *input);
  cv::Mat *processOutput(cv::Mat *img, cv::Mat *output, QString & Labels);
  void NMS(std::vector<Box> &boxes);           //非极大抑制
  float IOU(const Box &box1, const Box &box2); //算IOU
  void paintbox(cv::Mat &img, const std::vector<Box> &boxes);
  bool isValidDetection(const Box &box);       //检测框有效性验证
  void loadDetectionConfig();                  //加载检测配置
  static bool CmpConf(const Box &b1, const Box &b2) {
    return b1.confidence > b2.confidence;
  }

public:
  TorchEngine_DV(const QString &model);
  cv::Mat *infer(cv::Mat *tensor,QString & Labels);
};
