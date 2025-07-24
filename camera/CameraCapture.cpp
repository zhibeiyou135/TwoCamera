//
// Created by pe on 2020/7/14.
//

#include "CameraCapture.h"
#include <QDateTime>
#include <QDir>
#include "CropConfig.h"
#include "DVDisplayConfig.h"
#include "RecordingConfig.h"
#include "FileSaveManager.h"
#include <QElapsedTimer>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

using namespace cv;
// void __stdcall ImageCallBackEx(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
CameraCapture::CameraCapture()
    : running(false), recording(false), allowRecording(true) {}

// 加载配置文件
void CameraCapture::loadConfig(const QString &jsonPath) {
    // 加载DV显示配置
    loadDVDisplayConfigFromJson(jsonPath);
    
    // 加载录制配置
    loadRecordingConfigFromJson(jsonPath);
    
    // 加载海康相机配置
    loadHikvisionConfig(jsonPath);
    
    // 将配置应用到裁剪设置
    auto dvConfig = DVDisplayConfig::getInstance();
    auto cropConfig = CropConfig::getInstance();
    
    // 设置裁剪参数
    cropConfig->setEnableCrop(dvConfig->getCropEnabled());
    cropConfig->setX(dvConfig->getCropX());
    cropConfig->setY(dvConfig->getCropY());
    cropConfig->setWidth(dvConfig->getCropWidth());
    cropConfig->setHeight(dvConfig->getCropHeight());
    
    // 创建保存路径
    if (dvConfig->getSaveEnabled()) {
        QDir().mkpath(QString::fromStdString(dvConfig->getSavePath()));
    }
}

// 处理帧方法，根据配置决定如何处理视频帧
cv::Mat* CameraCapture::processFrame(cv::Mat& frame) {
    auto dvConfig = DVDisplayConfig::getInstance();
    Mat *result = new Mat();
    
    // 注意：此函数主要用于USB相机的备选方案
    // 对于海康相机，颜色转换在open()函数中的主循环里通过convertHikvisionColor()处理
    // 这里保持原有的OpenCV转换，因为USB相机需要这种转换
    cv::cvtColor(frame, *result, COLOR_BayerGR2BGR);
    
    // 检查配置的显示模式
    if (dvConfig->getDisplayMode() == "cropped" && dvConfig->getCropEnabled()) {
        // 裁剪已转换颜色的图像
        *result = cropImage(*result);
    }
    
    // 将颜色空间从BGR转换为RGB以便在Qt中显示
    cv::cvtColor(*result, *result, COLOR_BGR2RGB);
    
    return result;
}

// 裁剪图像方法
cv::Mat CameraCapture::cropImage(const cv::Mat& src) {
    auto config = CropConfig::getInstance();
    int x = config->getX();
    int y = config->getY();
    int width = config->getWidth();
    int height = config->getHeight();
    
    // 确保裁剪区域不超出原图范围
    x = std::max(0, std::min(x, src.cols - 1));
    y = std::max(0, std::min(y, src.rows - 1));
    width = std::min(width, src.cols - x);
    height = std::min(height, src.rows - y);
    
    cv::Rect roi(x, y, width, height);
    return src(roi).clone();
}

// 旋转图像方法，基于您提供的Python代码逻辑
cv::Mat CameraCapture::rotateImage(const cv::Mat& src, double angle) {
    if (std::abs(angle) < 0.1) {  // 如果角度很小，直接返回原图
        return src.clone();
    }
    
    int rows = src.rows;
    int cols = src.cols;
    cv::Point2f center(cols / 2.0, rows / 2.0);
    
    // 创建旋转矩阵
    cv::Mat rotationMatrix = cv::getRotationMatrix2D(center, angle, 1.0);
    
    // 根据配置选择边界填充模式
    auto dvConfig = DVDisplayConfig::getInstance();
    int borderMode = cv::BORDER_REFLECT_101; // 默认镜像填充
    std::string borderModeStr = dvConfig->getRotationBorderMode();
    
    if (borderModeStr == "constant") {
        borderMode = cv::BORDER_CONSTANT;
    } else if (borderModeStr == "reflect") {
        borderMode = cv::BORDER_REFLECT_101;
    } else if (borderModeStr == "replicate") {
        borderMode = cv::BORDER_REPLICATE;
    } else if (borderModeStr == "wrap") {
        borderMode = cv::BORDER_WRAP;
    }
    
    // 根据配置选择插值方法
    int interpolation = cv::INTER_LINEAR; // 默认线性插值
    std::string interpStr = dvConfig->getRotationInterpolation();
    
    if (interpStr == "nearest") {
        interpolation = cv::INTER_NEAREST;
    } else if (interpStr == "linear") {
        interpolation = cv::INTER_LINEAR;
    } else if (interpStr == "cubic") {
        interpolation = cv::INTER_CUBIC;
    } else if (interpStr == "lanczos4") {
        interpolation = cv::INTER_LANCZOS4;
    }
    
    cv::Mat rotated;
    cv::warpAffine(src, rotated, rotationMatrix, cv::Size(cols, rows), 
                   interpolation, borderMode);
    
    return rotated;
}

// 应用旋转和裁剪的组合方法
cv::Mat CameraCapture::applyRotationAndCrop(const cv::Mat& src) {
    auto dvConfig = DVDisplayConfig::getInstance();
    cv::Mat result = src.clone();
    
    // 1. 先应用旋转（如果启用）
    if (dvConfig->getRotationEnabled()) {
        double angle = dvConfig->getRotationAngle();
        result = rotateImage(result, angle);
        qDebug() << "应用旋转，角度:" << angle << "度";
    }
    
    // 2. 再应用裁剪（如果启用）
    if (dvConfig->getCropEnabled()) {
        result = cropImage(result);
        qDebug() << "应用裁剪，尺寸:" << result.rows << "x" << result.cols;
    }
    
    return result;
}

// 设置是否启用裁剪
void CameraCapture::setEnableCrop(bool enable) {
    CropConfig::getInstance()->setEnableCrop(enable);
    
    // 同步更新DVDisplayConfig
    DVDisplayConfig::getInstance()->setCropEnabled(enable);
}

// 设置裁剪参数
void CameraCapture::setCropParams(int x, int y, int width, int height) {
    auto config = CropConfig::getInstance();
    config->setX(x);
    config->setY(y);
    config->setWidth(width);
    config->setHeight(height);
    
    // 同步更新DVDisplayConfig
    auto dvConfig = DVDisplayConfig::getInstance();
    dvConfig->setCropX(x);
    dvConfig->setCropY(y);
    dvConfig->setCropWidth(width);
    dvConfig->setCropHeight(height);
}

// ch:像素排列由RGB转为BGR | en:Convert pixel arrangement from RGB to BGR

void CameraCapture::saveImageThread()
{
	while (running.load())
	
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		// printf("mdmdmmdmd");
		queueCondVar.wait(lock,[this](){return (!imageQueue.empty() || !cropImageQueue.empty()) || !running.load();});
		// if (!running.load())break;
		
		// 先处理原始图像
		if (!imageQueue.empty())
		{
			std::cout << "Processing original image, queue size: " << imageQueue.size() << std::endl;
			cv::Mat img = imageQueue.front();
			imageQueue.pop();
      std::string filename = nameQueue.front();
      nameQueue.pop();
      
			lock.unlock();
      cv::imwrite(filename, img);
		}
		// 处理裁剪图像
		else if (!cropImageQueue.empty())
		{
			std::cout << "Processing cropped image, queue size: " << cropImageQueue.size() << std::endl;
			cv::Mat img = cropImageQueue.front();
			cropImageQueue.pop();
      std::string filename = cropNameQueue.front();
      cropNameQueue.pop();
      
			lock.unlock();
      cv::imwrite(filename, img);
		}
	}
}
// ch:保存图片 | en:Save Image


bool CameraCapture::open(int index) {
  index = (index < 0) ? cameraIndex : index;

  // 优先尝试海康工业相机
  qDebug() << "正在尝试打开海康工业相机...";

  nRet = MV_CC_Initialize();
  MV_CC_DEVICE_INFO_LIST stDeviceList;
  memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
  nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_CAMERALINK_DEVICE | MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE, &stDeviceList);
  
  if (stDeviceList.nDeviceNum > 0) {
    qDebug() << "发现" << stDeviceList.nDeviceNum << "个海康相机设备";
    
    // 使用海康相机
    for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) {
                MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
      qDebug() << "海康相机设备[" << i << "]信息已获取";
    }  

    // 确保索引在有效范围内
    if (index >= static_cast<int>(stDeviceList.nDeviceNum)) {
      index = 0;
      qDebug() << "索引超出范围，使用第一个海康相机设备";
            }  
    
    nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[index]);
    if (nRet != MV_OK) {
      qDebug() << "海康相机创建句柄失败，错误码:" << nRet;
      goto try_usb_camera;
        } 
    
  nRet = MV_CC_OpenDevice(handle);
    if (nRet != MV_OK) {
      qDebug() << "海康相机打开设备失败，错误码:" << nRet;
      goto try_usb_camera;
    }
  
    qDebug() << "海康工业相机打开成功";

		// ch:获取数据包大小 | en:Get payload size
  MVCC_INTVALUE stParam;
  memset(&stParam, 0, sizeof(MVCC_INTVALUE));
  nRet = MV_CC_GetIntValue(handle, "PayloadSize", &stParam);

  unsigned int nPayloadSize = stParam.nCurValue;

  // ch:设置触发模式为off | en:Set trigger mode as off
  nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 0);

  // 应用海康相机曝光时间配置
  nRet = MV_CC_SetEnumValue(handle, "ExposureAuto", hikvisionExposureAuto);
  if (MV_OK != nRet) {
      qDebug() << "设置海康相机自动曝光模式失败，错误码:" << nRet;
  } else {
      QString modeStr = (hikvisionExposureAuto == 0) ? "关闭" : (hikvisionExposureAuto == 1) ? "一次" : "连续";
      qDebug() << "设置海康相机自动曝光模式成功:" << modeStr;
  }
  
  // 如果自动曝光关闭，设置手动曝光时间
  if (hikvisionExposureAuto == 0) {
      nRet = MV_CC_SetFloatValue(handle, "ExposureTime", hikvisionExposureTime);
      if (MV_OK != nRet) {
          qDebug() << "设置海康相机曝光时间失败，错误码:" << nRet;
      } else {
          qDebug() << "设置海康相机曝光时间成功:" << hikvisionExposureTime << "微秒";
      }
  }

  // 添加白平衡相关参数设置
  // 1. 设置自动白平衡模式
  nRet = MV_CC_SetEnumValue(handle, "BalanceWhiteAuto", 1); // 0:Off, 1:Once, 2:Continuous
  if (MV_OK != nRet) {
      qDebug() << "设置自动白平衡失败，错误码:" << nRet;
  } else {
      qDebug() << "设置自动白平衡成功";
  }

  // 2. 如果需要手动调整白平衡，可以设置具体的白平衡比例
  // 先关闭自动白平衡
  // nRet = MV_CC_SetEnumValue(handle, "BalanceWhiteAuto", 0);

  // // 设置红色通道增益 (通常范围 0.0-4.0, 默认1.0)
  // nRet = MV_CC_SetFloatValue(handle, "BalanceRatioRed", 0.8f);
  // if (MV_OK != nRet) {
  //     qDebug() << "设置红色白平衡增益失败，错误码:" << nRet;
  // }

  // // 设置绿色通道增益 (通常范围 0.0-4.0, 默认1.0)  
  // nRet = MV_CC_SetFloatValue(handle, "BalanceRatioGreen", 1.0f);
  // if (MV_OK != nRet) {
  //     qDebug() << "设置绿色白平衡增益失败，错误码:" << nRet;
  // }

  // // 设置蓝色通道增益 (通常范围 0.0-4.0, 默认1.0)
  // nRet = MV_CC_SetFloatValue(handle, "BalanceRatioBlue", 1.2f);
  // if (MV_OK != nRet) {
  //     qDebug() << "设置蓝色白平衡增益失败，错误码:" << nRet;
  // }

  // 3. 可选：设置色彩饱和度调整
  // nRet = MV_CC_SetFloatValue(handle, "Saturation", 1.0f); // 范围通常0.0-2.0
  // if (MV_OK != nRet) {
  //     qDebug() << "设置色彩饱和度失败，错误码:" << nRet;
  // }

  // 4. 可选：设置Gamma值调整
  // nRet = MV_CC_SetFloatValue(handle, "Gamma", 1.0f); // 范围通常0.1-4.0
  // if (MV_OK != nRet) {
  //     qDebug() << "设置Gamma值失败，错误码:" << nRet;
  // }

  nRet = MV_CC_StartGrabbing(handle);

    // 海康相机处理线程
  QFuture<bool> res = QtConcurrent::run([this]() -> bool {
    using namespace std;
    MV_FRAME_OUT stImageInfo = {0};
	MV_CC_HB_DECODE_PARAM stDecodeParam = {0};

    running.store(true);
      for (int i = 0; i < 15; i++) {
        saveThread[i] = std::thread(&CameraCapture::saveImageThread, this);
      }

    QDateTime current_date_time = QDateTime::currentDateTime();
    QDateTime last_date_time = QDateTime::currentDateTime();
    
      auto dvConfig = DVDisplayConfig::getInstance();
      
      QElapsedTimer frameTimer;
      frameTimer.start();
      const int targetFrameInterval = 50; // 50ms对应20fps，与配置一致
      
      // 添加检测帧率控制
      QElapsedTimer detectFrameTimer;
      detectFrameTimer.start();
      const int detectFrameInterval = 200; // 200ms对应5fps，减轻检测负担
      
      unsigned char *pBgrData = 0;
      for (;;) {
        nRet = MV_CC_GetImageBuffer(handle, &stImageInfo, 2000);
        if (nRet != MV_OK) {
          qDebug() << "海康相机获取图像缓冲区失败，错误码:" << nRet;
          continue;
        }
        
        cv::Mat f(stImageInfo.stFrameInfo.nExtendHeight, stImageInfo.stFrameInfo.nExtendWidth, CV_8UC1, stImageInfo.pBufAddr);
        nRet = MV_CC_FreeImageBuffer(handle, &stImageInfo);
        
        // 根据配置决定发送给检测和显示的图像
        Mat *r = nullptr;
        
        // 使用专用的海康相机颜色转换函数
        cv::Mat colorConverted = convertHikvisionColor(f, stImageInfo);
        
        // 应用翻转（如果启用）- 从dv配置中读取翻转参数
        auto dvConfig = DVDisplayConfig::getInstance();
        if (dvConfig) {
            // 通过DVDisplayConfig获取翻转设置（需要确保DVDisplayConfig支持翻转配置）
            // 或者直接调用applyFlip方法，它会自动读取配置
            colorConverted = applyFlip(colorConverted);
        }
        
        // 根据配置应用旋转和裁剪
        if (dvConfig->getDisplayMode() == "cropped" && 
            (dvConfig->getCropEnabled() || dvConfig->getRotationEnabled())) {
          // 应用旋转和裁剪的组合处理
          cv::Mat processed = applyRotationAndCrop(colorConverted);
          
          // 转换为RGB用于显示和检测
          r = new Mat();
          cv::cvtColor(processed, *r, COLOR_BGR2RGB);
          
          qDebug() << "海康相机: 发送旋转+裁剪图像给检测模块，尺寸:" << r->rows << "x" << r->cols;
        } else {
          // 发送原图（仅做颜色转换）
          r = new Mat();
          cv::cvtColor(colorConverted, *r, COLOR_BGR2RGB);
          qDebug() << "海康相机: 发送原图给检测模块，尺寸:" << r->rows << "x" << r->cols;
        }
        
        // 录制逻辑
        if (allowRecording.load(std::memory_order_relaxed) &&
            recording.load(std::memory_order_relaxed)) {
          if (frameTimer.elapsed() >= targetFrameInterval) {
            frameTimer.restart();
            uint64_t timeCount = std::chrono::duration_cast<std::chrono::microseconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count();
            
            auto recordingConfig = RecordingConfig::getInstance();
            
            // 保存原始图像（如果启用）- 使用FileSaveManager避免UI卡死
            if (recordingConfig->getSaveDVOriginal()) {
              std::string filename = pathdv.toStdString()+"/" +std::to_string(timeCount) +"_original.png";
              // 使用FileSaveManager异步保存，避免阻塞UI线程
              FileSaveManager::getInstance()->saveDVOriginal(colorConverted, QString::fromStdString(filename), timeCount);
              qDebug() << "海康相机: 已提交原始BGR图像保存任务";
            }
            
            // 保存裁剪图像（如果启用）- 使用FileSaveManager避免UI卡死
            if (recordingConfig->getSaveDVCropped()) {
              // 应用旋转和裁剪处理，使用已经正确转换的图像
              cv::Mat processed = applyRotationAndCrop(colorConverted);

              std::string cropFilename = pathCrop.toStdString() + "/" + std::to_string(timeCount) + "_cropped.png";
              // 使用FileSaveManager异步保存，避免阻塞UI线程
              FileSaveManager::getInstance()->saveDVCropped(processed, QString::fromStdString(cropFilename), timeCount);
              qDebug() << "海康相机: 已提交裁剪BGR图像保存任务";
            }
            
            queueCondVar.notify_one();
          }
        }
        
        emit captureImage(r);
      }
    });
    
    emit initFinished();
    return true;
    
  } else {
    qDebug() << "未发现海康相机设备，尝试使用USB摄像头作为备选...";
  }

try_usb_camera:
  // 如果海康相机失败，作为备选方案尝试USB摄像头
  qDebug() << "正在尝试打开USB摄像头作为备选方案...";
  vc.open(index, cv::CAP_V4L2);
  if (!vc.isOpened()) {
    qDebug() << "USB摄像头也无法打开，DV相机初始化失败";
    return false;
  }
  
  qDebug() << "USB摄像头打开成功（作为海康相机的备选方案）";
  vc.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
  vc.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
  vc.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
  
  QFuture<bool> res = QtConcurrent::run([this]() -> bool {
    using namespace std;
    Mat f;
    running.store(true);
    
    // 初始化保存线程
    for (int i = 0; i < 15; i++) {
      saveThread[i] = std::thread(&CameraCapture::saveImageThread, this);
    }
    
    auto dvConfig = DVDisplayConfig::getInstance();
    QElapsedTimer frameTimer;
    frameTimer.start();
    const int targetFrameInterval = 50; // 50ms对应20fps，与配置一致
    
    for (;;) {
      vc >> f;
      if (f.empty()) {
        qDebug() << "USB摄像头: 接收到空帧";
        continue;
      }
      
      // 录制逻辑
      if (allowRecording.load(std::memory_order_relaxed) &&
          recording.load(std::memory_order_relaxed)) {
        if (frameTimer.elapsed() >= targetFrameInterval) {
          frameTimer.restart();
          uint64_t timeCount = std::chrono::duration_cast<std::chrono::microseconds>(
                      std::chrono::system_clock::now().time_since_epoch()).count();
          
          auto recordingConfig = RecordingConfig::getInstance();
          
          // 保存原始图像（如果启用）- 使用FileSaveManager避免UI卡死
          if (recordingConfig->getSaveDVOriginal()) {
            // USB相机图像转换为RGB格式保存
            cv::Mat rgbImage;
            cv::cvtColor(f, rgbImage, COLOR_BGR2RGB);
            std::string filename = pathdv.toStdString()+"/" +std::to_string(timeCount) +"_original.png";
            // 使用FileSaveManager异步保存，避免阻塞UI线程
            FileSaveManager::getInstance()->saveDVOriginal(rgbImage, QString::fromStdString(filename), timeCount);
            qDebug() << "USB相机: 已提交原始RGB图像保存任务";
          }
          
          // 保存裁剪图像（如果启用）- 使用FileSaveManager避免UI卡死
          if (recordingConfig->getSaveDVCropped()) {
            // 应用旋转和裁剪处理
            cv::Mat processed = applyRotationAndCrop(f);

            // 转换为RGB格式保存
            cv::Mat rgbProcessed;
            cv::cvtColor(processed, rgbProcessed, COLOR_BGR2RGB);

            std::string cropFilename = pathCrop.toStdString() + "/" + std::to_string(timeCount) + "_cropped.png";
            // 使用FileSaveManager异步保存，避免阻塞UI线程
            FileSaveManager::getInstance()->saveDVCropped(rgbProcessed, QString::fromStdString(cropFilename), timeCount);
            qDebug() << "USB相机: 已提交裁剪RGB图像保存任务";
          }
          
          queueCondVar.notify_one();
        }
      }
      
      // 创建用于显示和检测的图像
      std::shared_ptr<cv::Mat> r = std::make_shared<cv::Mat>();
      cv::cvtColor(f, *r, COLOR_BGR2RGB);
      
      // 根据配置应用旋转和裁剪
      if (dvConfig->getDisplayMode() == "cropped" && 
          (dvConfig->getCropEnabled() || dvConfig->getRotationEnabled())) {
        // 先转换回BGR进行处理
        cv::Mat bgrImg;
        cv::cvtColor(*r, bgrImg, COLOR_RGB2BGR);
        
        // 应用旋转和裁剪的组合处理
        cv::Mat processed = applyRotationAndCrop(bgrImg);
        
        // 转换回RGB
        cv::cvtColor(processed, *r, COLOR_BGR2RGB);
        qDebug() << "USB相机: 应用旋转+裁剪，尺寸:" << r->rows << "x" << r->cols;
      }
      
      // 使用原始指针发射信号（为了兼容现有接口）
      cv::Mat* rawPtr = new cv::Mat(*r);
      emit captureImage(rawPtr);
    }
	});
  
  emit initFinished();
  return true;
}


void CameraCapture::startRecord(const QString &path) {
  auto recordingConfig = RecordingConfig::getInstance();
  
  // 修复路径后缀解析逻辑
  QString sessionSuffix = "";
  if (!path.isEmpty()) {
    // 对于自动录制，直接使用完整的路径作为后缀
    if (path.startsWith("auto_")) {
      sessionSuffix = path;  // 使用完整的 "auto_1_min" 格式
      qDebug() << "DV相机检测到自动录制格式，使用完整路径作为后缀:" << sessionSuffix;
    } else {
      // 对于其他格式，提取最后一部分作为后缀
      QStringList pathParts = path.split("_");
      if (pathParts.size() > 1) {
        sessionSuffix = pathParts.last();
      } else {
        sessionSuffix = path;
      }
      qDebug() << "DV相机普通录制格式，提取后缀:" << sessionSuffix;
    }
  }
  
  // 启动文件保存服务
  FileSaveManager::getInstance()->startService();

  // 创建新的录制会话
  QString sessionPath = recordingConfig->createRecordingSession(sessionSuffix);
  qDebug() << "Starting DV camera recording session at:" << sessionPath;
  
  // 设置各种保存路径
  if (recordingConfig->getSaveDVOriginal()) {
    pathdv = recordingConfig->getDVOriginalPath();
    qDebug() << "DV original images will be saved to:" << pathdv;
  }
  
  if (recordingConfig->getSaveDVCropped()) {
    pathCrop = recordingConfig->getDVCroppedPath();
    qDebug() << "DV cropped images will be saved to:" << pathCrop;
    }
  
  // 创建相机日志记录器（如果需要保存原图）
  if (recordingConfig->getSaveDVOriginal()) {
  cameraLogger = std::make_shared<CameraLogger>(pathdv.toStdString());
  }
  
  recording.store(true, std::memory_order_relaxed);
  
  qDebug() << "DV camera recording started with configuration:"
           << "\n  Session path:" << sessionPath
           << "\n  Save DV original:" << recordingConfig->getSaveDVOriginal()
           << "\n  Save DV cropped:" << recordingConfig->getSaveDVCropped()
           << "\n  DV original path:" << pathdv
           << "\n  DV cropped path:" << pathCrop;
}

void CameraCapture::saveImage(cv::Mat *img) { cameraLogger->addImage(img); }

void CameraCapture::stopRecord() {
  recording.store(false, std::memory_order_relaxed);
  
  // 清理当前录制会话路径，确保下次录制创建新的会话
  auto recordingConfig = RecordingConfig::getInstance();
  recordingConfig->clearCurrentSession();
  qDebug() << "DV recording stopped and cleared current session";
}

void CameraCapture::saveRecord(const QString &savePath) {}

void CameraCapture::startCapture(int index) { open(index); }

void CameraCapture::setSavePath(const QString &savePath) {
    // 保存路径
    QString path = savePath;
    if (path.isEmpty()) {
        path = "recordings";
    }
    
    // 确保路径存在
    QDir().mkpath(path);
    
    qDebug() << "设置DV图像保存路径为：" << path;
    
    // 这里可以添加任何其他特定于相机捕获的保存路径设置
    recordingPath = path;
}

// 添加白平衡调整方法的实现
void CameraCapture::setAutoWhiteBalance(bool enable) {
    if (handle == nullptr) {
        qDebug() << "相机句柄无效，无法设置白平衡";
        return;
    }
    
    int nRet = MV_CC_SetEnumValue(handle, "BalanceWhiteAuto", enable ? 2 : 0); // 2:Continuous, 0:Off
    if (MV_OK != nRet) {
        qDebug() << "设置自动白平衡失败，错误码:" << nRet;
    } else {
        qDebug() << "设置自动白平衡" << (enable ? "开启" : "关闭") << "成功";
    }
}

void CameraCapture::setWhiteBalanceRatio(float red, float green, float blue) {
    if (handle == nullptr) {
        qDebug() << "相机句柄无效，无法设置白平衡比例";
        return;
    }
    
    // 先关闭自动白平衡
    MV_CC_SetEnumValue(handle, "BalanceWhiteAuto", 0);
    
    int nRet;
    nRet = MV_CC_SetFloatValue(handle, "BalanceRatioRed", red);
    if (MV_OK != nRet) {
        qDebug() << "设置红色白平衡增益失败，错误码:" << nRet;
    }
    
    nRet = MV_CC_SetFloatValue(handle, "BalanceRatioGreen", green);
    if (MV_OK != nRet) {
        qDebug() << "设置绿色白平衡增益失败，错误码:" << nRet;
    }
    
    nRet = MV_CC_SetFloatValue(handle, "BalanceRatioBlue", blue);
    if (MV_OK != nRet) {
        qDebug() << "设置蓝色白平衡增益失败，错误码:" << nRet;
    } else {
        qDebug() << "设置白平衡比例成功 - R:" << red << " G:" << green << " B:" << blue;
    }
}

void CameraCapture::setSaturation(float saturation) {
    if (handle == nullptr) {
        qDebug() << "相机句柄无效，无法设置饱和度";
        return;
    }
    
    int nRet = MV_CC_SetFloatValue(handle, "Saturation", saturation);
    if (MV_OK != nRet) {
        qDebug() << "设置色彩饱和度失败，错误码:" << nRet;
    } else {
        qDebug() << "设置色彩饱和度成功:" << saturation;
    }
}

void CameraCapture::setGamma(float gamma) {
    if (handle == nullptr) {
        qDebug() << "相机句柄无效，无法设置Gamma";
        return;
    }
    
    int nRet = MV_CC_SetFloatValue(handle, "Gamma", gamma);
    if (MV_OK != nRet) {
        qDebug() << "设置Gamma值失败，错误码:" << nRet;
    } else {
        qDebug() << "设置Gamma值成功:" << gamma;
    }
}

// 添加颜色校正函数
cv::Mat CameraCapture::correctColor(const cv::Mat& src) {
    cv::Mat result;
    
    // 改进的颜色校正矩阵 - 更有效地减少黄色偏移
    // 黄色偏移通常是由于红色和绿色通道过强导致的
    cv::Mat colorMatrix = (cv::Mat_<float>(3, 3) << 
        0.95, -0.15,  0.05,   // 红色通道：减少红色，增加一点蓝色
        -0.05,  0.85,  0.0,   // 绿色通道：减少绿色强度
        0.1,   0.0,   1.15    // 蓝色通道：增强蓝色来平衡黄色
    );
    
    // 应用颜色校正
    src.convertTo(result, CV_32F, 1.0/255.0); // 转换到0-1范围
    
    // 分离通道 (OpenCV是BGR顺序)
    std::vector<cv::Mat> channels;
    cv::split(result, channels);
    
    // 应用颜色矩阵变换 (注意BGR顺序)
    cv::Mat newB = colorMatrix.at<float>(0,0) * channels[0] + 
                   colorMatrix.at<float>(0,1) * channels[1] + 
                   colorMatrix.at<float>(0,2) * channels[2];
    cv::Mat newG = colorMatrix.at<float>(1,0) * channels[0] + 
                   colorMatrix.at<float>(1,1) * channels[1] + 
                   colorMatrix.at<float>(1,2) * channels[2];
    cv::Mat newR = colorMatrix.at<float>(2,0) * channels[0] + 
                   colorMatrix.at<float>(2,1) * channels[1] + 
                   colorMatrix.at<float>(2,2) * channels[2];
    
    // 限制值范围到0-1
    cv::threshold(newB, newB, 0, 0, cv::THRESH_TOZERO);
    cv::threshold(newG, newG, 0, 0, cv::THRESH_TOZERO);
    cv::threshold(newR, newR, 0, 0, cv::THRESH_TOZERO);
    cv::threshold(newB, newB, 1, 1, cv::THRESH_TRUNC);
    cv::threshold(newG, newG, 1, 1, cv::THRESH_TRUNC);
    cv::threshold(newR, newR, 1, 1, cv::THRESH_TRUNC);
    
    // 重新组合通道
    std::vector<cv::Mat> newChannels = {newB, newG, newR};
    cv::merge(newChannels, result);
    
    // 转换回8位
    result.convertTo(result, CV_8U, 255.0);
    
    return result;
}

// 专用的海康相机颜色转换函数
cv::Mat CameraCapture::convertHikvisionColor(const cv::Mat& bayerImage, const MV_FRAME_OUT& frameInfo) {
    cv::Mat colorConverted;
    
    // 检查像素格式
    if (frameInfo.stFrameInfo.enPixelType == PixelType_Gvsp_BayerGR8) {
        // 使用海康SDK进行Bayer到BGR的转换
        MV_CC_PIXEL_CONVERT_PARAM stConvertParam = {0};
        memset(&stConvertParam, 0, sizeof(MV_CC_PIXEL_CONVERT_PARAM));
        
        // 计算转换后的图像大小
        unsigned int nBGRSize = frameInfo.stFrameInfo.nWidth * frameInfo.stFrameInfo.nHeight * 3;
        unsigned char* pBGRData = (unsigned char*)malloc(nBGRSize);
        
        if (pBGRData) {
            // 设置转换参数
            stConvertParam.nWidth = frameInfo.stFrameInfo.nWidth;
            stConvertParam.nHeight = frameInfo.stFrameInfo.nHeight;
            stConvertParam.pSrcData = frameInfo.pBufAddr;
            stConvertParam.nSrcDataLen = frameInfo.stFrameInfo.nFrameLen;
            stConvertParam.enSrcPixelType = frameInfo.stFrameInfo.enPixelType;
            stConvertParam.pDstBuffer = pBGRData;
            stConvertParam.nDstBufferSize = nBGRSize;
            stConvertParam.enDstPixelType = PixelType_Gvsp_BGR8_Packed;
            
            // 执行转换
            int nRet = MV_CC_ConvertPixelType(handle, &stConvertParam);
            if (MV_OK == nRet) {
                // 创建BGR格式的Mat，使用海康SDK的原始转换结果
                colorConverted = cv::Mat(frameInfo.stFrameInfo.nHeight, frameInfo.stFrameInfo.nWidth, CV_8UC3, pBGRData).clone();
                qDebug() << "海康SDK颜色转换成功，使用原始转换结果";
            } else {
                qDebug() << "海康SDK颜色转换失败，错误码:" << nRet << "，回退到OpenCV转换";
                cv::cvtColor(bayerImage, colorConverted, COLOR_BayerGR2BGR);
                // 对OpenCV转换的结果应用颜色校正
                colorConverted = correctColor(colorConverted);
            }
            
            free(pBGRData);
        } else {
            qDebug() << "分配内存失败，回退到OpenCV转换";
            cv::cvtColor(bayerImage, colorConverted, COLOR_BayerGR2BGR);
            colorConverted = correctColor(colorConverted);
        }
    } else {
        qDebug() << "不支持的像素格式，回退到OpenCV转换";
        cv::cvtColor(bayerImage, colorConverted, COLOR_BayerGR2BGR);
        colorConverted = correctColor(colorConverted);
    }
    
    return colorConverted;
}

// 加载海康相机配置
void CameraCapture::loadHikvisionConfig(const QString& jsonPath) {
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法打开配置文件:" << jsonPath << "，使用默认海康相机配置";
        return;
    }

    QByteArray jsonData = file.readAll();
    QJsonDocument document = QJsonDocument::fromJson(jsonData);
    
    if (document.isNull() || !document.isObject()) {
        qDebug() << "配置文件格式错误:" << jsonPath << "，使用默认海康相机配置";
        return;
    }

    QJsonObject rootObj = document.object();
    if (!rootObj.contains("dv") || !rootObj["dv"].isObject()) {
        qDebug() << "配置文件中缺少'dv'部分，使用默认配置";
        return;
    }

    QJsonObject dvObj = rootObj["dv"].toObject();
    
    // 检查是否有海康相机配置
    if (!dvObj.contains("hikvisionCamera") || !dvObj["hikvisionCamera"].isObject()) {
        qDebug() << "配置文件中缺少'dv.hikvisionCamera'部分，使用默认配置";
        return;
    }

    QJsonObject hikvisionObj = dvObj["hikvisionCamera"].toObject();
    
    // 读取曝光时间配置
    if (hikvisionObj.contains("exposureTime") && hikvisionObj["exposureTime"].isDouble()) {
        hikvisionExposureTime = hikvisionObj["exposureTime"].toDouble();
        qDebug() << "从dv配置读取海康相机曝光时间:" << hikvisionExposureTime << "微秒";
    }
    
    // 读取自动曝光模式配置
    if (hikvisionObj.contains("exposureAuto") && hikvisionObj["exposureAuto"].isDouble()) {
        hikvisionExposureAuto = hikvisionObj["exposureAuto"].toInt();
        qDebug() << "从dv配置读取海康相机自动曝光模式:" << hikvisionExposureAuto;
    }
    
    qDebug() << "海康相机配置加载完成，翻转功能使用dv配置中的horizontalFlip和verticalFlip";
}

// 应用翻转
cv::Mat CameraCapture::applyFlip(const cv::Mat& src) {
    cv::Mat result = src.clone();
    
    // 从dv配置中读取翻转参数
    QFile file("config.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法打开配置文件，使用默认翻转设置";
        return result;
    }

    QByteArray jsonData = file.readAll();
    QJsonDocument document = QJsonDocument::fromJson(jsonData);
    
    if (document.isNull() || !document.isObject()) {
        qDebug() << "配置文件格式错误，使用默认翻转设置";
        return result;
    }

    QJsonObject rootObj = document.object();
    if (!rootObj.contains("dv") || !rootObj["dv"].isObject()) {
        qDebug() << "配置文件中缺少dv部分，使用默认翻转设置";
        return result;
    }

    QJsonObject dvObj = rootObj["dv"].toObject();
    
    // 读取翻转配置
    bool horizontalFlip = false;
    bool verticalFlip = false;
    
    if (dvObj.contains("horizontalFlip") && dvObj["horizontalFlip"].isBool()) {
        horizontalFlip = dvObj["horizontalFlip"].toBool();
    }
    
    if (dvObj.contains("verticalFlip") && dvObj["verticalFlip"].isBool()) {
        verticalFlip = dvObj["verticalFlip"].toBool();
    }
    
    // 应用翻转
    if (horizontalFlip && verticalFlip) {
        // 同时翻转X和Y轴
        cv::flip(result, result, -1);
        qDebug() << "应用DV相机XY轴翻转";
    } else if (horizontalFlip) {
        // 只翻转X轴（水平翻转）
        cv::flip(result, result, 1);
        qDebug() << "应用DV相机X轴翻转";
    } else if (verticalFlip) {
        // 只翻转Y轴（垂直翻转）
        cv::flip(result, result, 0);
        qDebug() << "应用DV相机Y轴翻转";
    }
    
    return result;
}

// 设置曝光时间
void CameraCapture::setExposureTime(float exposureTime) {
    if (handle == nullptr) {
        qDebug() << "相机句柄无效，无法设置曝光时间";
        return;
    }
    
    hikvisionExposureTime = exposureTime;
    int nRet = MV_CC_SetFloatValue(handle, "ExposureTime", exposureTime);
    if (MV_OK != nRet) {
        qDebug() << "设置海康相机曝光时间失败，错误码:" << nRet;
    } else {
        qDebug() << "设置海康相机曝光时间成功:" << exposureTime << "微秒";
    }
}

// 设置自动曝光模式
void CameraCapture::setExposureAuto(int mode) {
    if (handle == nullptr) {
        qDebug() << "相机句柄无效，无法设置自动曝光模式";
        return;
    }
    
    hikvisionExposureAuto = mode;
    int nRet = MV_CC_SetEnumValue(handle, "ExposureAuto", mode);
    if (MV_OK != nRet) {
        qDebug() << "设置海康相机自动曝光模式失败，错误码:" << nRet;
    } else {
        QString modeStr = (mode == 0) ? "关闭" : (mode == 1) ? "一次" : "连续";
        qDebug() << "设置海康相机自动曝光模式成功:" << modeStr;
    }
}

// 设置X轴翻转
void CameraCapture::setFlipX(bool enable) {
    // 更新dv配置中的horizontalFlip参数
    QFile file("config.json");
    if (!file.open(QIODevice::ReadWrite)) {
        qDebug() << "无法打开配置文件进行X轴翻转设置";
        return;
    }

    QByteArray jsonData = file.readAll();
    QJsonDocument document = QJsonDocument::fromJson(jsonData);
    
    if (document.isNull() || !document.isObject()) {
        qDebug() << "配置文件格式错误，无法设置X轴翻转";
        file.close();
        return;
    }

    QJsonObject rootObj = document.object();
    if (!rootObj.contains("dv") || !rootObj["dv"].isObject()) {
        qDebug() << "配置文件中缺少dv部分，无法设置X轴翻转";
        file.close();
        return;
    }

    QJsonObject dvObj = rootObj["dv"].toObject();
    dvObj["horizontalFlip"] = enable;
    rootObj["dv"] = dvObj;
    
    // 写回配置文件
    document.setObject(rootObj);
    file.seek(0);
    file.write(document.toJson());
    file.resize(file.pos());
    file.close();
    
    qDebug() << "设置DV相机X轴翻转:" << (enable ? "开启" : "关闭");
}

// 设置Y轴翻转
void CameraCapture::setFlipY(bool enable) {
    // 更新dv配置中的verticalFlip参数
    QFile file("config.json");
    if (!file.open(QIODevice::ReadWrite)) {
        qDebug() << "无法打开配置文件进行Y轴翻转设置";
        return;
    }

    QByteArray jsonData = file.readAll();
    QJsonDocument document = QJsonDocument::fromJson(jsonData);
    
    if (document.isNull() || !document.isObject()) {
        qDebug() << "配置文件格式错误，无法设置Y轴翻转";
        file.close();
        return;
    }

    QJsonObject rootObj = document.object();
    if (!rootObj.contains("dv") || !rootObj["dv"].isObject()) {
        qDebug() << "配置文件中缺少dv部分，无法设置Y轴翻转";
        file.close();
        return;
    }

    QJsonObject dvObj = rootObj["dv"].toObject();
    dvObj["verticalFlip"] = enable;
    rootObj["dv"] = dvObj;
    
    // 写回配置文件
    document.setObject(rootObj);
    file.seek(0);
    file.write(document.toJson());
    file.resize(file.pos());
    file.close();
    
    qDebug() << "设置DV相机Y轴翻转:" << (enable ? "开启" : "关闭");
}
