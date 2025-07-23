//
// Created by pe on 2021/5/6.
//

#include "DVSDataSource.h"
#include "DVSBiasConfig.h"
#include "camera/RecordingConfig.h"
#include "camera/ConfigManager.h"
#include <QColor>
#include <QImage>
#include <QQueue>
#include <QtConcurrent/QtConcurrent>
#include <QElapsedTimer>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <chrono>
#include <memory>
#include <thread>
using namespace cv;
using namespace std::chrono;


// #define CELEX_IMG_HEIGHT 720
// #define CELEX_IMG_WIDTH 1280

extern bool enableDetect;

// 为了防止frame_gen在初始化后被销毁，我们将它作为类的静态成员
std::shared_ptr<Metavision::PeriodicFrameGenerationAlgorithm> DVSDataSource::frameGenerator = nullptr;
std::thread DVSDataSource::postProcessThread;
bool DVSDataSource::postProcessThreadStarted = false;

// 析构函数 - 确保清理所有资源
DVSDataSource::~DVSDataSource() {
  // 停止相机
  stopCamera();
  
  // 停止录制（如果正在进行）
  if (recordFlag.load()) {
    stopRecord();
  }
  
  // 停止图像保存线程
  qDebug() << "Stopping image save threads...";
    {
    std::lock_guard<std::mutex> lock(queueMutex);
    for (auto &thread : saveThread) {
      if (thread.joinable()) {
        thread.detach();  // 使用detach避免阻塞
      }
    }
  }
  
  qDebug() << "DVSDataSource destroyed";
}

void DVSDataSource::addData(QByteArray data) {
  // 此函数保留，但不再用于RAW录制，而是用于处理自定义数据
  if (recordFlag.load() && enableSaveRaw) {
    // 数据将通过Metavision SDK的log_raw_data自动保存
    // 不需要在这里手动写入文件
  }
}

QImage paintColor(cv::Mat &img) {
  using namespace cv;
  using namespace std;
  static auto element = getStructuringElement(MORPH_RECT, Size(8, 8));
  cv::Mat denoise;
  medianBlur(img, denoise, 3);
  cv::Mat res;
  dilate(denoise, res, element);
  vector<Mat> channels(3);
  cv::bitwise_and(img, res, channels[0]);
  cv::bitwise_xor(channels[0], img, channels[1]);
  cv::bitwise_xor(channels[0], img, channels[2]);
  cv::bitwise_or(channels[0], img, channels[0]);
  Mat *imgRes = new Mat;
  merge(channels, *imgRes);
  return QImage(
      imgRes->data, imgRes->cols, imgRes->rows, imgRes->step,
      QImage::Format_RGB888, [](void *p) { delete (Mat *)p; }, imgRes);
}

QImage shared_ptrToQImage(std::shared_ptr<cv::Mat> img) {
  auto ptr = new std::shared_ptr<cv::Mat>(img);
  return QImage(
      img->data, img->cols, img->rows, img->step, QImage::Format_Grayscale8,
      [](void *p) { delete (std::shared_ptr<cv::Mat> *)p; }, ptr);
}

DVSDataSource::DVSDataSource() {
  enableSaveRaw.store(true);
  recordFlag.store(false);
  autoStart.store(false);
  cameraInitialized.store(false);
  cameraRunning.store(false);
  
  // 初始化图像缓冲区
  imgBuffer = new cv::Mat(DVS_IMG_HEIGHT, DVS_IMG_WIDTH, CV_8UC1, cv::Scalar(0));
  
  qDebug() << "DVSDataSource initialized with autoStart:" << autoStart.load();
  
  // 只初始化，不启动相机处理
  QtConcurrent::run([this]() { this->initCamera(); });
}

// 初始化相机但不启动
void DVSDataSource::initCamera() {
  std::lock_guard<std::mutex> lock(cameraMutex);
  
  if (cameraInitialized.load()) {
    qDebug() << "Camera already initialized";
    return;
  }
  
  qDebug() << "Initializing camera...";
  
  try {
    // 创建相机
    try {
      cam = Metavision::Camera::from_first_available();
      qDebug() << "Camera created successfully";
    } catch (const std::exception &e) {
      qDebug() << "Error creating camera:" << e.what();
      return;
    }
    
    // 获取相机几何信息
    int camera_width = 1280;
    int camera_height = 720;
    
    // 尝试从相机获取实际几何信息
    try {
      auto geometry = cam.geometry();
      camera_width = geometry.width();
      camera_height = geometry.height();
      qDebug() << "Camera geometry:" << camera_width << "x" << camera_height;
    } catch (const std::exception &e) {
      qDebug() << "Could not retrieve camera geometry, using default:" << camera_width << "x" << camera_height;
    }
    
    // 从配置文件读取累积时间和帧率，如果读取失败则使用默认值
    std::uint32_t acc = 20000;  // 默认20ms，对应配置文件中的accumulationTime: 20
    double fps = 20.0;  // 默认20fps
    
    // 尝试从配置文件读取累积时间和帧率
    try {
      QString configPath = ConfigManager::getInstance().getConfigPath();
      QFile configFile(configPath);
      if (configFile.open(QIODevice::ReadOnly)) {
        QByteArray jsonData = configFile.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (doc.isObject()) {
          QJsonObject rootObj = doc.object();
          if (rootObj.contains("dvs") && rootObj["dvs"].isObject()) {
            QJsonObject dvsObj = rootObj["dvs"].toObject();
            
            // 读取累积时间
            if (dvsObj.contains("accumulationTime") && dvsObj["accumulationTime"].isDouble()) {
              int accumulationTimeMs = dvsObj["accumulationTime"].toInt();
              acc = accumulationTimeMs * 1000;  // 转换为微秒
              qDebug() << "从配置文件读取DVS累积时间:" << accumulationTimeMs << "ms (" << acc << "微秒)";
            }
            
            // 读取帧率
            if (dvsObj.contains("fps") && dvsObj["fps"].isDouble()) {
              fps = dvsObj["fps"].toDouble();
              qDebug() << "从配置文件读取DVS帧率:" << fps << "fps";
            }
          }
        }
      }
    } catch (const std::exception &e) {
      qDebug() << "读取DVS配置失败，使用默认值:" << e.what();
      qDebug() << "默认累积时间:" << acc << "微秒，默认帧率:" << fps << "fps";
    }
    
    // 设置当前帧率
    currentFPS.store(fps);
    
    // 创建帧生成算法，使用静态共享指针确保其生命周期
    frameGenerator = std::make_shared<Metavision::PeriodicFrameGenerationAlgorithm>(camera_width, camera_height, acc, fps);
    
    // 设置事件回调
    cam.cd().add_callback([this](const Metavision::EventCD *begin, const Metavision::EventCD *end) {
      if (!frameGenerator) return;  // 安全检查
      
      try {
        // 处理事件
        event_analyzer.analyze_events(begin, end);
        
        // 传递事件给帧生成器
        frameGenerator->process_events(begin, end);
      } catch (const std::exception &e) {
        qDebug() << "Error processing events:" << e.what();
      }
    });
    
    // 设置帧回调
    frameGenerator->set_output_callback([this](Metavision::timestamp ts, cv::Mat &frame) {
      try {
        // 保存图像（如果需要）
        if (recordFlag.load() && enableSaveImg.load()) {
          cv::Mat frameCopy = frame.clone();  // 复制帧以避免数据竞争
          std::lock_guard<std::mutex> lock(queueMutex);
          imageQueue.push(frameCopy);
          queueCondVar.notify_one();
        }
        
        // 创建QImage并发送信号
        QImage frameQImage = QImage(frame.data, frame.cols, frame.rows, 
                                  frame.step, QImage::Format_RGB888).copy();
        
        qDebug() << "Emitting DVS image: " << frameQImage.width() << "x" << frameQImage.height();
        emit newImage(frameQImage);
      } catch (const std::exception &e) {
        qDebug() << "Error in frame callback:" << e.what();
      }
    });
    
    // 启动图像保存线程
    for (int i = 0; i < 15; i++) {  // 减少线程数量，避免资源消耗过大
      saveThread[i] = std::thread(&DVSDataSource::saveImageThread, this);
    }
    
    // 只启动一次后处理线程
    if (!postProcessThreadStarted) {
      postProcessThread = std::thread(&DVSDataSource::postProcess, this);
      postProcessThreadStarted = true;
    }
    
    cameraInitialized.store(true);
    qDebug() << "Camera initialized successfully";
    
    // 加载并应用偏置设置
    loadBiasSettings();
    applyBiasSettings();
    
    // 如果设置了自动启动，则启动相机
    if (autoStart.load()) {
      qDebug() << "Auto-starting camera...";
      startCamera();
    } else {
      qDebug() << "Camera initialized but not started (autoStart is false)";
    }
  } catch (const std::exception &e) {
    qDebug() << "Error initializing camera:" << e.what();
  }
}

// 后处理函数，包含轮询但不启动相机
void DVSDataSource::postProcess() {
  qDebug() << "Starting post-process loop";
  
  while (true) {
    try {
      if (cameraRunning.load()) {
        // 轮询Metavision事件
        Metavision::EventLoop::poll_and_dispatch(20);  // 20ms轮询间隔
      } else {
        // 相机未运行时，降低CPU使用率
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    } catch (const std::exception &e) {
      qDebug() << "Error in post-process loop:" << e.what();
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));  // 错误时暂停一下
    }
  }
}

// 明确的相机启动方法
void DVSDataSource::startCamera() {
  std::lock_guard<std::mutex> lock(cameraMutex);
  
  if (!cameraInitialized.load()) {
    qDebug() << "Camera not initialized yet, initializing...";
    initCamera();
    
    if (!cameraInitialized.load()) {
      qDebug() << "Failed to initialize camera";
      return;
    }
  }
  
  if (cameraRunning.load()) {
    qDebug() << "Camera already running";
    return;
  }
  
  qDebug() << "Starting camera...";
  
  try {
    // 启动相机
    cam.start();
    cameraRunning.store(true);
    qDebug() << "Camera started successfully";
    
    // 发送信号通知相机已启动
    emit cameraStarted();
  } catch (const std::exception &e) {
    qDebug() << "Error starting camera:" << e.what();
  }
}

// 明确的相机停止方法
void DVSDataSource::stopCamera() {
  std::lock_guard<std::mutex> lock(cameraMutex);
  
  if (!cameraRunning.load()) {
    qDebug() << "Camera not running";
    return;
  }
  
  qDebug() << "Stopping camera...";
  
  try {
    // 停止相机
    cam.stop();
    cameraRunning.store(false);
    qDebug() << "Camera stopped successfully";
    
    // 发送信号通知相机已停止
    emit cameraStopped();
  } catch (const std::exception &e) {
    qDebug() << "Error stopping camera:" << e.what();
  }
}

// 停止录制
void DVSDataSource::stopRecord() {
  if (!recordFlag.load()) {
    qDebug() << "Recording not active";
    return;
  }
  
  qDebug() << "Stopping DVS recording...";
  
  try {
    // 使用Metavision SDK停止RAW文件录制
    if (cameraRunning.load() && enableSaveRaw.load()) {
      qDebug() << "Stopping RAW file recording";
      cam.stop_recording();
      
      // 等待录制完全停止
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // 重置所有录制相关状态
    recordFlag.store(false);
    enableSaveRaw.store(false);
    enableSaveImg.store(false);
    rawFilePath.clear();
    saveFolderPath.clear();
    
    // 清理当前录制会话路径，确保下次录制创建新的会话
    auto recordingConfig = RecordingConfig::getInstance();
    recordingConfig->clearCurrentSession();
    qDebug() << "Cleared current recording session";
    
    qDebug() << "DVS recording stopped successfully";
  } catch (const std::exception &e) {
    qDebug() << "Error stopping DVS recording:" << e.what();
    // 即使出错也要重置状态
    recordFlag.store(false);
    enableSaveRaw.store(false);
    enableSaveImg.store(false);
    
    // 即使出错也要清理会话
    auto recordingConfig = RecordingConfig::getInstance();
    recordingConfig->clearCurrentSession();
  }
}

// 开始录制
void DVSDataSource::startRecord(const QString &path) {
  qDebug() << "Starting DVS record with path:" << path;
  
  // 如果正在录制，先停止并等待
  if (recordFlag.load()) {
    qDebug() << "DVS recording already active, stopping previous recording";
    stopRecord();
    
    // 等待停止完成
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }
  
  auto recordingConfig = RecordingConfig::getInstance();
  
  // 修复路径后缀解析逻辑
  QString sessionSuffix = "";
  if (!path.isEmpty()) {
    // 对于自动录制，直接使用完整的路径作为后缀
    if (path.startsWith("auto_")) {
      sessionSuffix = path;  // 使用完整的 "auto_1_min" 格式
      qDebug() << "检测到自动录制格式，使用完整路径作为后缀:" << sessionSuffix;
    } else {
      // 对于其他格式，提取最后一部分作为后缀
      QStringList pathParts = path.split("_");
      if (pathParts.size() > 1) {
        sessionSuffix = pathParts.last();
      } else {
        sessionSuffix = path;
      }
      qDebug() << "普通录制格式，提取后缀:" << sessionSuffix;
    }
  }
  
  // 总是创建新的录制会话，与DV相机保持一致的行为
  QString sessionPath = recordingConfig->createRecordingSession(sessionSuffix);
  qDebug() << "DVS recording session path:" << sessionPath;
  
  // 设置RAW数据保存路径（如果启用）
  if (recordingConfig->getSaveDVSRaw()) {
    // 生成唯一的RAW文件名，包含时间戳
    uint64_t timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    QString rawPath = recordingConfig->getDVSRawPath() + "/dvs_" + QString::number(timestamp) + ".raw";
    rawFilePath = rawPath.toStdString();
    qDebug() << "DVS RAW file path:" << rawPath;
    
    try {
      // 确保目录存在
      QDir().mkpath(recordingConfig->getDVSRawPath());
      
      // 保存偏置设置文件到同一目录
      QString biasFilePath = recordingConfig->getDVSRawPath() + "/dvs.bias";
      saveBiasToFile(biasFilePath);
      
      // 使用Metavision SDK开始RAW文件录制
      if (cameraRunning.load()) {
        qDebug() << "Starting DVS RAW file recording at:" << rawPath;
        cam.start_recording(rawFilePath);
        enableSaveRaw.store(true);
        qDebug() << "DVS RAW recording started successfully";
      } else {
        qDebug() << "DVS camera not running, cannot start RAW recording";
        return;  // 如果相机没运行就不要继续
      }
    } catch (const std::exception &e) {
      qDebug() << "Error starting DVS RAW recording:" << e.what();
      return;  // 如果RAW录制失败就不要继续
    }
  }

  // 设置图像保存路径（如果启用）
  if (recordingConfig->getSaveDVSImages()) {
    saveFolderPath = recordingConfig->getDVSImagesPath().toStdString();
    
    // 确保目录存在
    QDir().mkpath(recordingConfig->getDVSImagesPath());
    
    enableSaveImg.store(true);
    qDebug() << "DVS images will be saved to:" << QString::fromStdString(saveFolderPath);
  }
  
  // 最后设置录制标志
  recordFlag.store(true);
  
  qDebug() << "DVS recording started successfully with configuration:"
           << "\n  Session path:" << sessionPath
           << "\n  Save DVS images:" << recordingConfig->getSaveDVSImages()
           << "\n  Save DVS raw:" << recordingConfig->getSaveDVSRaw()
           << "\n  DVS images path:" << QString::fromStdString(saveFolderPath)
           << "\n  DVS raw path:" << QString::fromStdString(rawFilePath);
}

void DVSDataSource::setDenoise(bool enable) { enableDenoise.store(enable); }

void DVSDataSource::setSpeed(int b) { beilv.store(b); }

void DVSDataSource::setOverlapCount(int o) { overLapCount.store(o); }

void DVSDataSource::syncImage() { syncImageFlag.store(true); }

// 图像保存线程
void DVSDataSource::saveImageThread()
{
	while (true)
	
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		// printf("mdmdmmdmd");
		queueCondVar.wait(lock,[this](){return !imageQueue.empty();});
		// if (!running.load())break;
		if (!imageQueue.empty())
		{
			std::cout << "DVS saving image, queue size: " << imageQueue.size() << std::endl;
			cv::Mat img = imageQueue.front();
			imageQueue.pop();
			lock.unlock();
			
			// 只有在录制状态且启用图像保存时才保存
			if (recordFlag.load() && enableSaveImg.load()) {
		  uint64_t timeCount =
            duration_cast<microseconds>(
                system_clock::now().time_since_epoch())
                .count();

				cv::imwrite(saveFolderPath + "/"  + std::to_string(timeCount) +"_dvs.png", img);
			}
		}
	}
}

void DVSDataSource::loadFpn(const QString &fpn) {
  QFile file(fpn);
  if (!file.open(QFile::ReadOnly)) {
    qDebug() << "fpn not found" << endl;
    exit(0);
  }
  QTextStream ts(file.readAll());
  for (int i = 0; i < 1280 * 720; i++) {
    ts >> fpnData[i];
  }
}

// 偏置设置相关方法实现

/**
 * @brief 从配置文件加载偏置设置
 */
void DVSDataSource::loadBiasSettings() {
  qDebug() << "Loading DVS bias settings from configuration...";
  
  try {
    QString configPath = ConfigManager::getInstance().getConfigPath();
    QFile configFile(configPath);
    
    if (!configFile.open(QIODevice::ReadOnly)) {
      qDebug() << "Cannot open config file for bias settings:" << configPath;
      return;
    }
    
    QByteArray jsonData = configFile.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    
    if (!doc.isObject()) {
      qDebug() << "Invalid JSON format in config file";
      return;
    }
    
    QJsonObject rootObj = doc.object();
    if (!rootObj.contains("dvs") || !rootObj["dvs"].isObject()) {
      qDebug() << "No DVS configuration found in config file";
      return;
    }
    
    QJsonObject dvsObj = rootObj["dvs"].toObject();
    if (!dvsObj.contains("biases") || !dvsObj["biases"].isObject()) {
      qDebug() << "No bias configuration found in DVS section";
      return;
    }
    
    QJsonObject biasObj = dvsObj["biases"].toObject();
    
    // 加载偏置配置到管理器
    DVSBiasManager::getInstance().loadFromJson(biasObj);
    
    qDebug() << "DVS bias settings loaded successfully";
    
  } catch (const std::exception &e) {
    qDebug() << "Error loading bias settings:" << e.what();
  }
}

/**
 * @brief 应用偏置设置到DVS相机
 */
bool DVSDataSource::applyBiasSettings() {
  qDebug() << "Applying DVS bias settings to camera...";
  
  try {
    const DVSBiasConfig& config = DVSBiasManager::getInstance().getBiasConfig();
    
    if (!config.enabled) {
      qDebug() << "Bias settings disabled, using default values";
      return true;
    }
    
    // 验证偏置值范围
    if (!config.validateBiases()) {
      qDebug() << "Bias values validation failed, using default values";
      return false;
    }
    
    // 获取相机的偏置设施
    auto i_ll_biases = cam.get_device().get_facility<Metavision::I_LL_Biases>();
    if (!i_ll_biases) {
      qDebug() << "Camera does not support bias configuration";
      return false;
    }
    
    // 应用偏置设置
    auto biasMap = config.getBiasMap();
    bool allSuccessful = true;
    
    for (const auto& [biasName, biasValue] : biasMap) {
      try {
        if (i_ll_biases->set(biasName, biasValue)) {
          qDebug() << "Successfully set" << biasName.c_str() << "to" << biasValue;
        } else {
          qDebug() << "Failed to set" << biasName.c_str() << "to" << biasValue;
          allSuccessful = false;
        }
      } catch (const std::exception &e) {
        qDebug() << "Exception setting" << biasName.c_str() << ":" << e.what();
        allSuccessful = false;
      }
    }
    
    if (allSuccessful) {
      qDebug() << "All bias settings applied successfully";
    } else {
      qDebug() << "Some bias settings failed to apply";
    }
    
    return allSuccessful;
    
  } catch (const std::exception &e) {
    qDebug() << "Error applying bias settings:" << e.what();
    return false;
  }
}

/**
 * @brief 保存当前偏置设置到文件
 */
bool DVSDataSource::saveBiasToFile(const QString& filePath) {
  qDebug() << "Saving current bias settings to file:" << filePath;
  
  try {
    // 获取相机的偏置设施
    auto i_ll_biases = cam.get_device().get_facility<Metavision::I_LL_Biases>();
    if (!i_ll_biases) {
      qDebug() << "Camera does not support bias configuration";
      return false;
    }
    
    // 确保目录存在
    QDir().mkpath(QFileInfo(filePath).absolutePath());
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      qDebug() << "Cannot open file for writing:" << filePath;
      return false;
    }
    
    QTextStream out(&file);
    
    // 写入偏置文件头部注释
    out << "# DVS Camera Bias Settings\n";
    out << "# Generated on " << QDateTime::currentDateTime().toString() << "\n";
    out << "# Format: value % bias_name\n";
    out << "\n";
    
    // 获取并保存所有偏置值
    std::vector<std::string> biasNames = {
      "bias_diff", "bias_diff_off", "bias_diff_on", 
      "bias_fo", "bias_hpf", "bias_refr"
    };
    
    for (const auto& biasName : biasNames) {
      try {
        int value = i_ll_biases->get(biasName);
        out << value << "    % " << biasName.c_str() << "\n";
        qDebug() << "Saved" << biasName.c_str() << "=" << value;
      } catch (const std::exception &e) {
        qDebug() << "Error reading" << biasName.c_str() << ":" << e.what();
        out << "0    % " << biasName.c_str() << " # Error reading value\n";
      }
    }
    
    file.close();
    qDebug() << "Bias settings saved successfully to" << filePath;
    return true;
    
  } catch (const std::exception &e) {
    qDebug() << "Error saving bias settings:" << e.what();
    return false;
  }
}

// 设置DVS帧率
void DVSDataSource::setFPS(double fps) {
  if (fps <= 0) {
    qDebug() << "Invalid FPS value:" << fps << ", ignoring";
    return;
  }
  
  currentFPS.store(fps);
  qDebug() << "DVS帧率设置为:" << fps << "fps";
  
  // 如果相机已经初始化，需要重新创建帧生成器
  if (cameraInitialized.load() && frameGenerator) {
    try {
      // 获取当前相机几何信息
      int camera_width = 1280;
      int camera_height = 720;
      
      if (cam.geometry().width() > 0 && cam.geometry().height() > 0) {
        camera_width = cam.geometry().width();
        camera_height = cam.geometry().height();
      }
      
      // 从配置文件读取累积时间
      std::uint32_t acc = 20000;  // 默认20ms
      try {
        QString configPath = ConfigManager::getInstance().getConfigPath();
        QFile configFile(configPath);
        if (configFile.open(QIODevice::ReadOnly)) {
          QByteArray jsonData = configFile.readAll();
          QJsonDocument doc = QJsonDocument::fromJson(jsonData);
          if (doc.isObject()) {
            QJsonObject rootObj = doc.object();
            if (rootObj.contains("dvs") && rootObj["dvs"].isObject()) {
              QJsonObject dvsObj = rootObj["dvs"].toObject();
              if (dvsObj.contains("accumulationTime") && dvsObj["accumulationTime"].isDouble()) {
                int accumulationTimeMs = dvsObj["accumulationTime"].toInt();
                acc = accumulationTimeMs * 1000;  // 转换为微秒
              }
            }
          }
        }
      } catch (const std::exception &e) {
        qDebug() << "读取累积时间失败，使用默认值:" << e.what();
      }
      
      // 重新创建帧生成器
      frameGenerator = std::make_shared<Metavision::PeriodicFrameGenerationAlgorithm>(camera_width, camera_height, acc, fps);
      
      // 重新设置帧回调
      frameGenerator->set_output_callback([this](Metavision::timestamp ts, cv::Mat &frame) {
        try {
          // 保存图像（如果需要）
          if (recordFlag.load() && enableSaveImg.load()) {
            cv::Mat frameCopy = frame.clone();  // 复制帧以避免数据竞争
            std::lock_guard<std::mutex> lock(queueMutex);
            imageQueue.push(frameCopy);
            queueCondVar.notify_one();
          }
          
          // 创建QImage并发送信号
          QImage frameQImage = QImage(frame.data, frame.cols, frame.rows, 
                                    frame.step, QImage::Format_RGB888).copy();
          
          emit newImage(frameQImage);
        } catch (const std::exception &e) {
          qDebug() << "Error in frame callback:" << e.what();
        }
      });
      
      qDebug() << "DVS帧生成器已更新为新的帧率:" << fps << "fps";
    } catch (const std::exception &e) {
      qDebug() << "更新DVS帧率失败:" << e.what();
    }
  }
}

// 获取当前DVS帧率
double DVSDataSource::getFPS() const {
  return currentFPS.load();
}