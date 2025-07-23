#include "./TorchEngine_DVS.h"
#ifdef TORCH_ENABLE
#include <torch/script.h>
// 移除torch/torch.h以避免Python依赖
#endif
#include <QDebug>
#include <QFileInfo>
#include <mutex>

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
  if (boxes.size() > 0) {
    int top = 0; // top为当前轮次最好的box
    std::vector<Box> boxes_temp;
    while (1) {
      //保留历史所有最好的box
      boxes_temp.assign(boxes.begin(), boxes.begin() + top + 1);

      for (auto itr = boxes.begin() + top + 1; itr < boxes.end(); itr++) {
        // top为当前轮次最好的box
        //最好的box和下面的box比
        if (IOU(*(boxes.begin() + top), *itr) < DVS_IOU_THRE)
          boxes_temp.push_back(*itr);
      }
      boxes.swap(boxes_temp);
      boxes_temp.clear();
      top++;
      if (boxes.begin() + top == boxes.end())
        break;
    }
    qDebug() << "DVS box num after: " << boxes.size();
  }
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
    if (output->at<float>(i, 4) > DVS_CONF_THRE && output->at<float>(i, 4) < 1) {
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
      boxes.push_back(box);
    }
  }

  //使用NMS前需要降序排序
  std::sort(boxes.begin(), boxes.end(), CmpConf);
  qDebug() << "DVS box num before: " << boxes.size();
  if(boxes.size()<1)
    Labels+="无";
  NMS(boxes);
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