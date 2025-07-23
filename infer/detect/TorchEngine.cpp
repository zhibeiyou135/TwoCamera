#include "./TorchEngine.h"
#ifdef TORCH_ENABLE
#include <torch/script.h>
#endif
#include <QDebug>


TorchEngine::TorchEngine(const QString &model) {
#ifdef TORCH_ENABLE
  this->model = torch::jit::load(model.toStdString());
  this->model.to(at::kCUDA);
  
#endif
}

cv::Mat *TorchEngine::processInput(cv::Mat *input) {

  cv::Mat *cvt_out=new cv::Mat();
  cv::Mat *resize_out =new cv::Mat();
  cv::resize(*input, *resize_out, cv::Size(WIDTH, HEIGHT)); // resize
  resize_out->convertTo(*cvt_out, CV_32FC1, 1.0 / 255.0);       //归一化
  return cvt_out;
}


void TorchEngine::NMS(std::vector<Box> &boxes) {
  if (boxes.size() > 0) {
    int top = 0; // top为当前轮次最好的box
    std::vector<Box> boxes_temp;
    while (1) {
      //保留历史所有最好的box
      boxes_temp.assign(boxes.begin(), boxes.begin() + top + 1);

      for (auto itr = boxes.begin() + top + 1; itr < boxes.end(); itr++) {
        // top为当前轮次最好的box
        //最好的box和下面的box比
        if (IOU(*(boxes.begin() + top), *itr) < IOU_THRE)
          boxes_temp.push_back(*itr);
      }
      boxes.swap(boxes_temp);
      boxes_temp.clear();
      top++;
      // std::cout<<(boxes.begin()+top==boxes.end());
      if (boxes.begin() + top == boxes.end())
        break;
    }
    qDebug() << "box num after: " << boxes.size();
  }
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

    if (output->at<float>(i, 4) > CONF_THRE && output->at<float>(i, 4) < 1) {
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
      boxes.push_back(box);

      // left,top,right,bottom,conf*cls
    }
  }

  //使用NMS前需要降序排序
  std::sort(boxes.begin(), boxes.end(), CmpConf);
  /*
  for(Box box:boxes)
  {
      std::cout<<box.left<<std::endl;
      std::cout<<box.top<<std::endl;
      std::cout<<box.right<<std::endl;
      std::cout<<box.bottom<<std::endl<<std::endl;
  }*/
  qDebug() << "box num before: " << boxes.size();
  if(boxes.size()<1)
    Labels+="无";
  NMS(boxes);
  paintbox(*img, boxes);
  //cv::imshow("img",*img);
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
