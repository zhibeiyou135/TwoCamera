//
// Created by pe on 2020/7/14.
//

#ifndef TWOCAMERA_CAMERACAPTURE_H
#define TWOCAMERA_CAMERACAPTURE_H
#include <thread> 
#include <queue> 
#include <mutex> 
#include "BlockingQueue.h"
#include "CameraLogger.h"
#include <QAtomicInt>
#include <QFuture>
#include <QImage>
#include <QObject>
#include <QSharedPointer>
#include <QtConcurrent/QtConcurrent>
#include <atomic>
#include <opencv2/opencv.hpp>
#include <tuple>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "MvCameraControl.h"
#include "CropConfig.h"
#include "DVDisplayConfig.h"

// 声明JSON加载函数
void loadDVDisplayConfigFromJson(const QString& jsonPath);

// void __stdcall ImageCallBackEx(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser);

class CameraCapture : public QObject {
  Q_OBJECT
signals:

  void captureImage(cv::Mat *image);
  void captureCroppedImage(cv::Mat *image);

  void initFinished();

public:
  static CameraCapture *getInstance() {
    static CameraCapture instance;
    return &instance;
  }
  CameraCapture();


public slots:

  void startCapture(int index = -1);

  void startRecord(const QString &savePath);

  void stopRecord();

  void saveRecord(const QString &savePath);

  void setCameraIndex(int index) { cameraIndex = index; }

  void setAllowRecording(bool flag) {
    allowRecording.store(flag, std::memory_order_relaxed);
  }
  bool getAllowRecording() { return allowRecording.load(); }
  
  // 裁剪图像的配置方法
  void setEnableCrop(bool enable);
  void setCropParams(int x, int y, int width, int height);
  
  // 初始化配置
  void loadConfig(const QString &jsonPath = "detect.json");

  void setSavePath(const QString &savePath);

  // 白平衡相关方法
  void setAutoWhiteBalance(bool enable);
  void setWhiteBalanceRatio(float red, float green, float blue);
  void setSaturation(float saturation);
  void setGamma(float gamma);

  // 海康相机曝光时间和翻转相关方法
  void setExposureTime(float exposureTime);
  void setExposureAuto(int mode);
  void setFlipX(bool enable);
  void setFlipY(bool enable);

private:
  std::queue<cv::Mat> imageQueue; // 队列，用于存储需要保存的图片 
  std::queue<std::string> nameQueue; // 文件名队列
  std::queue<cv::Mat> cropImageQueue; // 裁剪图像队列
  std::queue<std::string> cropNameQueue; // 裁剪图像文件名队列
  std::mutex queueMutex; // 互斥量，用于保护队列的线程安全 
  std::condition_variable queueCondVar; // 条件变量，用于通知保存线程有新图片 
  std::thread saveThread[15]; // 保存线程
  void saveImageThread();
  
  // 记录路径
  QString recordingPath = "recordings";
  
  // 添加USB摄像头支持
  cv::VideoCapture vc;
  std::shared_ptr<CameraLogger> cameraLogger;
  int cameraIndex = 0;
  std::atomic_bool running, recording, allowRecording;
  QString pathdv;
  QString pathCrop; // 裁剪图像的保存路径
  void saveImage(cv::Mat *img);
  
  bool open(int index = -1);

  int SaveImage(MV_SAVE_IAMGE_TYPE enSaveImageType,MV_FRAME_OUT* stImageInfo);
  bool IsHBPixelFormat(MvGvspPixelType ePixelType);
  int nRet = MV_OK;
  unsigned char * pDstBuf = NULL; 
  void* handle = NULL;
  
  cv::Mat cropImage(const cv::Mat& src); // 裁剪图像方法
  cv::Mat* processFrame(cv::Mat& frame); // 处理帧方法，根据配置决定显示原图或裁剪图
  cv::Mat rotateImage(const cv::Mat& src, double angle); // 旋转图像方法
  cv::Mat applyRotationAndCrop(const cv::Mat& src); // 应用旋转和裁剪的组合方法
  cv::Mat correctColor(const cv::Mat& src); // 颜色校正方法
  cv::Mat convertHikvisionColor(const cv::Mat& bayerImage, const MV_FRAME_OUT& frameInfo); // 海康相机专用颜色转换
  
  // 海康相机配置相关
  void loadHikvisionConfig(const QString& jsonPath); // 加载海康相机配置
  cv::Mat applyFlip(const cv::Mat& src); // 应用翻转
  
  // 海康相机配置参数
  float hikvisionExposureTime = 10000.0f; // 曝光时间（微秒）
  int hikvisionExposureAuto = 0; // 自动曝光模式：0-关闭，1-一次，2-连续
  // 翻转参数已整合到dv配置中，不再单独维护
};

#endif // TWOCAMERA_CAMERACAPTURE_H
