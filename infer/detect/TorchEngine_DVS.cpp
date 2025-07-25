#include "./TorchEngine_DVS.h"
#ifdef TORCH_ENABLE
#include <torch/script.h>
// 移除torch/torch.h以避免Python依赖
#endif
#include <QDebug>
#include <QFileInfo>
#include <mutex>
#include <set>
#include <algorithm>

// 全局初始化锁，确保PyTorch只初始化一次（DVS共享同一个初始化状态）
extern std::mutex pytorch_init_mutex;
extern bool pytorch_initialized;

TorchEngine_DVS::TorchEngine_DVS(const QString &model) {
#ifdef TORCH_ENABLE
  qDebug() << "DVS模型路径:" << model;

  // 检查模型文件是否存在
  QFileInfo modelFile(model);
  if (!modelFile.exists()) {
    qDebug() << "错误: DVS模型文件不存在:" << model;
    throw std::runtime_error("DVS模型文件不存在");
  }

  if (modelFile.size() == 0) {
    qDebug() << "错误: DVS模型文件为空:" << model;
    throw std::runtime_error("DVS模型文件为空");
  }

  qDebug() << "DVS模型文件大小:" << modelFile.size() << "字节";

  // 加载检测配置
  loadDetectionConfig();
  
  // 全局PyTorch初始化（线程安全，与DV共享）
  {
    std::lock_guard<std::mutex> lock(pytorch_init_mutex);
    if (!pytorch_initialized) {
      try {
        qDebug() << "初始化PyTorch多线程设置...";
        qDebug() << "强制使用CUDA模式";
        pytorch_initialized = true;
        qDebug() << "PyTorch初始化完成";
      } catch (const std::exception& e) {
        qDebug() << "PyTorch初始化失败:" << e.what();
        throw;
      }
    }
  }
  
  try {
    qDebug() << "开始加载DVS模型...";
    this->model = torch::jit::load(model.toStdString());
    qDebug() << "DVS模型加载成功";
    
    // 设置模型为评估模式
    this->model.eval();
    
    // 强制使用CUDA模式
    qDebug() << "将DVS模型移动到CUDA...";
    this->model.to(at::kCUDA);
    qDebug() << "DVS模型CUDA设置完成";
    
    // 简化预热过程
    try {
      qDebug() << "预热DVS模型...";
      std::vector<torch::jit::IValue> dummy_inputs;
      torch::Tensor dummy_tensor = torch::randn({1, 3, HEIGHT, WIDTH});
      dummy_tensor = dummy_tensor.to(at::kCUDA);
      dummy_inputs.push_back(dummy_tensor);
      
      // 执行一次推理预热
      auto dummy_output = this->model.forward(dummy_inputs);
      qDebug() << "DVS模型预热完成";
    } catch (const std::exception& e) {
      qDebug() << "DVS模型预热失败（非致命错误）:" << e.what();
    }
    
  } catch (const std::exception& e) {
    qDebug() << "DVS模型加载失败:" << e.what();
    throw;
  } catch (...) {
    qDebug() << "DVS模型加载失败: 未知异常";
    throw;
  }
#else
  qDebug() << "TORCH_ENABLE未定义，跳过DVS模型加载";
  throw std::runtime_error("PyTorch支持未启用");
#endif
}

cv::Mat *TorchEngine_DVS::processInput(cv::Mat *input) {
  // 检查输入图像是否有效
  if (!input || input->empty() || input->data == nullptr) {
    qDebug() << "TorchEngine_DVS: 输入图像为空，返回空指针";
    return nullptr;
  }
  
  try {
    cv::Mat cvt_out;
    cv::Mat *resize_out = new cv::Mat();
    
    // 确保输入图像的内存是有效的
    if (input->isContinuous()) {
      input->convertTo(cvt_out, CV_32FC3, 1.0 / 255.0);       // 归一化
      cv::resize(cvt_out, *resize_out, cv::Size(WIDTH, HEIGHT)); // resize
    } else {
      // 如果内存不连续，先创建连续的副本
      cv::Mat continuous_input = input->clone();
      continuous_input.convertTo(cvt_out, CV_32FC3, 1.0 / 255.0);
      cv::resize(cvt_out, *resize_out, cv::Size(WIDTH, HEIGHT));
    }
    
    return resize_out;
  } catch (const std::exception& e) {
    qDebug() << "TorchEngine_DVS processInput 异常:" << e.what();
    return nullptr;
  } catch (...) {
    qDebug() << "TorchEngine_DVS processInput 未知异常";
    return nullptr;
  }
}

void TorchEngine_DVS::NMS(std::vector<Box> &boxes) {
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
  qDebug() << "DVS Enhanced NMS result: " << boxes.size() << "boxes";
}

float TorchEngine_DVS::IOU(const Box &box1, const Box &box2) {
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

cv::Mat *TorchEngine_DVS::processOutput(cv::Mat *img, cv::Mat *output, QString & Labels) {
  int left;
  int top;
  int right;
  int bottom;
  float clsconf;
  float max = 0;
  int max_id = -1;

  Box box{};
  std::vector<Box> boxes;
  
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
             DVS_HUGE_STEP * max_id;
      top = output->at<float>(i, 1) - output->at<float>(i, 3) / 2 +
            DVS_HUGE_STEP * max_id;
      right = output->at<float>(i, 0) + output->at<float>(i, 2) / 2 +
              DVS_HUGE_STEP * max_id;
      bottom = output->at<float>(i, 1) + output->at<float>(i, 3) / 2 +
               DVS_HUGE_STEP * max_id;
      clsconf = max * output->at<float>(i, 4);
      Labels="DVS识别结果:"+QString::fromStdString(name[max_id])+" ";
      box = Box{left, top, right, bottom, max_id, clsconf};

      // 应用高级过滤
      if (isValidDetection(box)) {
        boxes.push_back(box);
      }
    }
  }

  //使用NMS前需要降序排序
  std::sort(boxes.begin(), boxes.end(), CmpConf);
  qDebug() << "DVS box num before NMS: " << boxes.size();

  if(boxes.size()<1)
    Labels+="无";
  else {
    NMS(boxes);
    qDebug() << "DVS最终检测数量:" << boxes.size();
  }

  paintbox(*img, boxes);
  return img;
}

void TorchEngine_DVS::paintbox(cv::Mat &img, const std::vector<Box> &boxes) {
  for (Box box : boxes) {
    qDebug() << "DVS检测类别:" << box.cls;
    cv::Scalar color{static_cast<double>(color_list[box.cls][0]), 
                     static_cast<double>(color_list[box.cls][1]),
                     static_cast<double>(color_list[box.cls][2])};
    cv::rectangle(img,
                  cv::Point(box.left % DVS_HUGE_STEP * DVS_WIDTH_SCALE,
                            box.top % DVS_HUGE_STEP * DVS_HEIGHT_SCALE-50),
                  cv::Point(box.right % DVS_HUGE_STEP * DVS_WIDTH_SCALE,
                            box.bottom % DVS_HUGE_STEP * DVS_HEIGHT_SCALE-50),
                  color, 10);
  }
}

cv::Mat *TorchEngine_DVS::infer(cv::Mat *tensor, QString & Labels) {
  if (!tensor || tensor->empty()) {
    Labels = "DVS识别结果:输入图像无效";
    return nullptr;
  }
  
  cv::Mat *data = nullptr;
  try {
    data = processInput(tensor);
    if (!data) {
      Labels = "DVS识别结果:预处理失败";
      return tensor;
    }
    
    int size[2]{OUTMAPNUM, 5+CLASSNUM};
    cv::Mat outmap(2, size, CV_32F);
    
#ifdef TORCH_ENABLE
    try {
      torch::Tensor tensor_image = torch::from_blob(data->data, {1, HEIGHT, WIDTH, CHANNEL}, torch::kFloat);
      tensor_image = tensor_image.permute({0, 3, 1, 2});
      
      // 强制使用CUDA
      tensor_image = tensor_image.to(at::kCUDA);
      
      // 执行推理
      at::Tensor out = model.forward({tensor_image}).toTuple()->elements()[0].toTensor().to(torch::kCPU).detach().squeeze(0);
      
      // 安全的内存拷贝
      if (out.numel() * sizeof(float) <= outmap.total() * outmap.elemSize()) {
        memcpy(outmap.data, out.data_ptr(), out.numel() * sizeof(float));
        processOutput(tensor, &outmap, Labels);
      } else {
        qDebug() << "输出tensor大小不匹配，跳过后处理";
        Labels = "DVS识别结果:输出大小错误";
      }
      
    } catch (const std::exception& e) {
      qDebug() << "DVS推理过程异常:" << e.what();
      Labels = "DVS识别结果:推理异常";
    } catch (...) {
      qDebug() << "DVS推理过程未知异常";
      Labels = "DVS识别结果:推理异常";
    }
#else
    Labels = "DVS识别结果:TORCH未启用";
#endif
    
    // 安全删除处理后的数据
    if (data) {
      delete data;
      data = nullptr;
    }
    
    return tensor;
  } catch (const std::exception& e) {
    qDebug() << "TorchEngine_DVS infer 异常:" << e.what();
    Labels = "DVS识别结果:推理异常";
    
    // 清理内存
    if (data) {
      delete data;
      data = nullptr;
    }
    
    return tensor;
  } catch (...) {
    qDebug() << "TorchEngine_DVS infer 未知异常";
    Labels = "DVS识别结果:未知异常";

    // 清理内存
    if (data) {
      delete data;
      data = nullptr;
    }

    return tensor;
  }
}

// 加载检测配置
void TorchEngine_DVS::loadDetectionConfig() {
  try {
    QString configPath = ConfigManager::getInstance().getConfigPath();
    QFile configFile(configPath);

    if (!configFile.open(QIODevice::ReadOnly)) {
      qDebug() << "无法打开配置文件，使用默认DVS检测配置:" << configPath;
      return;
    }

    QByteArray jsonData = configFile.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);

    if (!doc.isObject()) {
      qDebug() << "配置文件格式错误，使用默认DVS检测配置";
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

    // 加载DVS配置
    if (parametersObj.contains("dvs") && parametersObj["dvs"].isObject()) {
      QJsonObject dvsConfig = parametersObj["dvs"].toObject();
      detectionConfig.loadFromJson(dvsConfig, "TorchEngine_DVS");
    } else {
      qDebug() << "未找到dvs配置，使用默认值";
    }

  } catch (const std::exception& e) {
    qDebug() << "加载DVS检测配置时发生异常:" << e.what();
  }
}

// 检测框有效性验证
bool TorchEngine_DVS::isValidDetection(const Box &box) {
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
    const float img_width = WIDTH * DVS_WIDTH_SCALE;
    const float img_height = HEIGHT * DVS_HEIGHT_SCALE;

    if (box.left < detectionConfig.border_margin ||
        box.top < detectionConfig.border_margin ||
        box.right > img_width - detectionConfig.border_margin ||
        box.bottom > img_height - detectionConfig.border_margin) {
      return false;
    }
  }

  return true;
}