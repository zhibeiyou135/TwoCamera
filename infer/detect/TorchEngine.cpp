#include "./TorchEngine.h"
#ifdef TORCH_ENABLE
#include <torch/script.h>
#endif
#include <QDebug>
#include <set>
#include <algorithm>


TorchEngine::TorchEngine(const QString &model) {
#ifdef TORCH_ENABLE
  this->model = torch::jit::load(model.toStdString());
  this->model.to(at::kCUDA);
#endif
  // 加载检测配置
  loadDetectionConfig();
}

cv::Mat *TorchEngine::processInput(cv::Mat *input) {

  cv::Mat *cvt_out=new cv::Mat();
  cv::Mat *resize_out =new cv::Mat();
  cv::resize(*input, *resize_out, cv::Size(WIDTH, HEIGHT)); // resize
  resize_out->convertTo(*cvt_out, CV_32FC1, 1.0 / 255.0);       //归一化
  return cvt_out;
}


void TorchEngine::NMS(std::vector<Box> &boxes) {
  if (boxes.empty()) {
    return;
  }

  // 增强的NMS算法 - 支持类别感知的NMS
  std::vector<Box> result;
  std::vector<bool> suppressed(boxes.size(), false);

  // 对每个类别分别进行NMS
  std::set<int> classes;
  for (const auto& box : boxes) {
    classes.insert(box.cls);
  }

  for (int cls : classes) {
    // 收集当前类别的所有检测框
    std::vector<std::pair<int, float>> class_boxes; // index, confidence
    for (size_t i = 0; i < boxes.size(); i++) {
      if (boxes[i].cls == cls && !suppressed[i]) {
        class_boxes.push_back({static_cast<int>(i), boxes[i].confidence});
      }
    }

    // 按置信度降序排序
    std::sort(class_boxes.begin(), class_boxes.end(),
              [](const std::pair<int, float>& a, const std::pair<int, float>& b) {
                return a.second > b.second;
              });

    // 对当前类别执行NMS
    for (size_t i = 0; i < class_boxes.size(); i++) {
      int idx_i = class_boxes[i].first;
      if (suppressed[idx_i]) continue;

      result.push_back(boxes[idx_i]);

      // 抑制与当前框重叠度高的其他框
      for (size_t j = i + 1; j < class_boxes.size(); j++) {
        int idx_j = class_boxes[j].first;
        if (suppressed[idx_j]) continue;

        float iou = IOU(boxes[idx_i], boxes[idx_j]);
        if (iou >= detectionConfig.iou_threshold) {
          suppressed[idx_j] = true;
        }
      }
    }
  }

  boxes = result;
  qDebug() << "Enhanced NMS result: " << boxes.size() << "boxes";
}
float TorchEngine::IOU(const Box &box1, const Box &box2) {
  int interBox[] = {
      std::max(box1.left, box2.left),     // left
      std::max(box1.top, box2.top),       // top
      std::min(box1.right, box2.right),   // right
      std::min(box1.bottom, box2.bottom), // bottom
  };

  float s1 = (box1.right - box1.left) * (box1.bottom - box1.top);
  float s2 = (box2.right - box2.left) * (box2.bottom - box2.top);
  // if top>bottom 或者 left>right
  if (interBox[1] > interBox[3] || interBox[0] > interBox[2])
    return 0.0f;
  float interBoxS = (interBox[2] - interBox[0]) * (interBox[3] - interBox[1]);
  return interBoxS / (s1 + s2 - interBoxS);
}
cv::Mat *TorchEngine::processOutput(cv::Mat *img, cv::Mat *output,QString & Labels) {
  int left;
  int top;
  int right;
  int bottom;
  float clsconf;
  float max = 0;
  int max_id = -1;

  Box box{};
  std::vector<Box> boxes;
  //qDebug()<<"output[0,0]:"<<output->at<float>(0,0);
  //检查置信度，筛选置信度高的
  for (int i = 0; i < output->rows; i++) {

    if (output->at<float>(i, 4) > detectionConfig.confidence_threshold && output->at<float>(i, 4) < 1) {
      max = output->at<float>(i, 5);
      max_id = 0;
      for (int j = 6; j < output->cols; j++) {
        if (output->at<float>(i, j) > max) {
          max = output->at<float>(i, j);
          max_id = j - 5;
        }
      }
      // xywh -> xyxy+cls
      left = output->at<float>(i, 0) - output->at<float>(i, 2) / 2 +
             HUGE_STEP * max_id;
      top = output->at<float>(i, 1) - output->at<float>(i, 3) / 2 +
            HUGE_STEP * max_id;
      right = output->at<float>(i, 0) + output->at<float>(i, 2) / 2 +
              HUGE_STEP * max_id;
      bottom = output->at<float>(i, 1) + output->at<float>(i, 3) / 2 +
               HUGE_STEP * max_id;
      clsconf = max * output->at<float>(i, 4);
      Labels="识别结果:"+QString::fromStdString(name[max_id]);
      box = Box{left, top, right, bottom, max_id, clsconf};

      // 应用高级过滤
      if (isValidDetection(box)) {
        boxes.push_back(box);
      }

      // left,top,right,bottom,conf*cls
    }
  }

  //使用NMS前需要降序排序
  std::sort(boxes.begin(), boxes.end(), CmpConf);
  qDebug() << "box num before NMS: " << boxes.size();

  if(boxes.size()<1)
    Labels+="无";
  else {
    NMS(boxes);
    qDebug() << "最终检测数量:" << boxes.size();
  }

  paintbox(*img, boxes);
  return img;
}
void TorchEngine::paintbox(cv::Mat &img, const std::vector<Box> &boxes) {
  for (Box box : boxes) {
    qDebug() << "检测类别:" << box.cls;
    
    //cv::cvtColor(img,img,CV_GRAY2BGR);
    cv::Scalar color{static_cast<double>(color_list[box.cls][0]), 
                     static_cast<double>(color_list[box.cls][1]),
                     static_cast<double>(color_list[box.cls][2])};
    cv::rectangle(img,
                  cv::Point(box.left % HUGE_STEP * WIDTH_SCALE,
                            box.top % HUGE_STEP * HEIGHT_SCALE-50),
                  cv::Point(box.right % HUGE_STEP * WIDTH_SCALE,
                            box.bottom % HUGE_STEP * HEIGHT_SCALE-50),
                  color, 10);
    // cv::putText(img, name[box.cls],
    //             cv::Point(box.left % HUGE_STEP * WIDTH_SCALE,
    //                       box.top % HUGE_STEP * HEIGHT_SCALE-70),
    //             cv::FONT_HERSHEY_COMPLEX, 5, cv::Scalar(255, 0, 0), 3);
  }
}
cv::Mat *TorchEngine::infer(cv::Mat *tensor,QString & Labels) {

  auto data = processInput(tensor);
  //int size[2]{1890, 11};
  int size[2]{OUTMAPNUM,5+CLASSNUM};
  cv::Mat outmap(2, size, CV_32F);

#ifdef TORCH_ENABLE
  //torch::Tensor tensor_image = torch::from_blob(data->data, {1, 288, 448, 3}, torch::kFloat);

  torch::Tensor tensor_image = torch::from_blob(data->data, {1, HEIGHT, WIDTH, CHANNEL}, torch::kFloat);
  tensor_image = tensor_image.permute({0, 3, 1, 2}).to(at::kCUDA); //输入
  at::Tensor out = model.forward({tensor_image}).toTuple()->elements()[0].toTensor().to(torch::kCPU).detach().squeeze(0);
  //u版yolo输出是list，如果输出不是list就使用下面这句
  //at::Tensor out = model.forward({tensor_image}).toTensor().to(torch::kCPU).detach().squeeze(0);
  memcpy(outmap.data, out.data_ptr(), out.numel() * sizeof(float));
  processOutput(tensor, &outmap, Labels);
  delete data;
#endif
  return tensor;
}

// 加载检测配置
void TorchEngine::loadDetectionConfig() {
  try {
    QString configPath = ConfigManager::getInstance().getConfigPath();
    QFile configFile(configPath);

    if (!configFile.open(QIODevice::ReadOnly)) {
      qDebug() << "无法打开配置文件，使用默认检测配置:" << configPath;
      return;
    }

    QByteArray jsonData = configFile.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);

    if (!doc.isObject()) {
      qDebug() << "配置文件格式错误，使用默认检测配置";
      return;
    }

    QJsonObject rootObj = doc.object();
    if (!rootObj.contains("detection") || !rootObj["detection"].isObject()) {
      qDebug() << "配置文件中未找到detection配置，使用默认值";
      return;
    }

    QJsonObject detectionObj = rootObj["detection"].toObject();
    if (!detectionObj.contains("parameters") || !detectionObj["parameters"].isObject()) {
      qDebug() << "配置文件中未找到detection.parameters配置，使用默认值";
      return;
    }

    QJsonObject parametersObj = detectionObj["parameters"].toObject();

    // 根据引擎类型选择配置（这里是基础TorchEngine，可能用于通用检测）
    QString engineType = "general";
    if (parametersObj.contains(engineType) && parametersObj[engineType].isObject()) {
      QJsonObject engineConfig = parametersObj[engineType].toObject();
      detectionConfig.loadFromJson(engineConfig, "TorchEngine");
    } else {
      qDebug() << "未找到" << engineType << "配置，使用默认值";
    }

  } catch (const std::exception& e) {
    qDebug() << "加载检测配置时发生异常:" << e.what();
  }
}

// 检测框有效性验证
bool TorchEngine::isValidDetection(const Box &box) {
  // 计算检测框尺寸
  float width = static_cast<float>(box.right - box.left);
  float height = static_cast<float>(box.bottom - box.top);
  float area = width * height;

  // 基本有效性检查
  if (width <= 0 || height <= 0) {
    return false;
  }

  // 尺寸过滤
  if (detectionConfig.enable_size_filter) {
    if (width < detectionConfig.min_width || width > detectionConfig.max_width ||
        height < detectionConfig.min_height || height > detectionConfig.max_height) {
      return false;
    }
  }

  // 面积过滤
  if (detectionConfig.enable_area_filter) {
    if (area < detectionConfig.min_area || area > detectionConfig.max_area) {
      return false;
    }
  }

  // 宽高比过滤
  if (detectionConfig.enable_aspect_filter && height > 0) {
    float aspect_ratio = width / height;
    if (aspect_ratio < detectionConfig.min_aspect_ratio ||
        aspect_ratio > detectionConfig.max_aspect_ratio) {
      return false;
    }
  }

  // 置信度分层过滤
  if (detectionConfig.enable_confidence_layers) {
    // 高置信度检测框放宽其他条件
    if (box.confidence >= detectionConfig.high_conf_threshold) {
      return true; // 高置信度直接通过
    }
    // 中等置信度需要满足更严格的条件
    else if (box.confidence >= detectionConfig.medium_conf_threshold) {
      // 中等置信度需要满足更严格的面积和宽高比要求
      if (area < detectionConfig.min_area * 1.5f || area > detectionConfig.max_area * 0.8f) {
        return false;
      }
    }
    // 低置信度需要满足最严格的条件
    else {
      // 低置信度需要满足最严格的要求
      if (area < detectionConfig.min_area * 2.0f || area > detectionConfig.max_area * 0.6f) {
        return false;
      }
      float aspect_ratio = width / height;
      if (aspect_ratio < detectionConfig.min_aspect_ratio * 1.2f ||
          aspect_ratio > detectionConfig.max_aspect_ratio * 0.8f) {
        return false;
      }
    }
  }

  // 边界过滤 - 检测框不能太靠近图像边界
  if (detectionConfig.enable_border_filter) {
    // 这里假设图像尺寸，实际应该从配置或参数传入
    const float img_width = WIDTH * WIDTH_SCALE;
    const float img_height = HEIGHT * HEIGHT_SCALE;

    if (box.left < detectionConfig.border_margin ||
        box.top < detectionConfig.border_margin ||
        box.right > img_width - detectionConfig.border_margin ||
        box.bottom > img_height - detectionConfig.border_margin) {
      return false;
    }
  }

  return true;
}
