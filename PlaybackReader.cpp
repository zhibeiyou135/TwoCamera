//
// Created by pe on 2021/4/22.
//

#include "PlaybackReader.h"
#include "dvs/DVSDataSource.h"
#include "camera/ConfigManager.h"
#include <QBitmap>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QList>
#include <QTextStream>
#include <QTime>
#include <QDateTime>
#include <QThread>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>
#include <deque>
#include <fstream>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <sstream>
#include <regex>
#include <QElapsedTimer>
#include <chrono>
#include <QTimer>

#include <metavision/sdk/driver/camera.h>
#include <metavision/sdk/base/events/event_cd.h>
#include <metavision/sdk/core/algorithms/periodic_frame_generation_algorithm.h>

using namespace std::chrono;

#define CELEX5_ROW 800

PlaybackReader::PlaybackReader() {
  // 初始化变量
  running.store(false);
  m_fileType = FileType::UNKNOWN;
}

PlaybackReader::~PlaybackReader() {
  if (running.load()) {
    stopPlayback();
  }
}

void PlaybackReader::setFilePath(const QString &filePath) {
  m_filePath = filePath;
  
  // 通过扩展名判断文件类型，只支持RAW格式
  QFileInfo fileInfo(filePath);
  QString extension = fileInfo.suffix().toLower();

  if (extension == "raw") {
    m_fileType = FileType::RAW;
    qDebug() << "File type set to RAW";
  } else {
    m_fileType = FileType::UNKNOWN;
    qDebug() << "Unsupported file type:" << extension << "- Only RAW files are supported";
  }
}

QString PlaybackReader::getFilePath() const {
  return m_filePath;
}

void PlaybackReader::setDVFolderPath(const QString &folderPath) {
  m_dvFolderPath = folderPath;
  qDebug() << "DV folder path set to:" << folderPath;
}

QString PlaybackReader::getDVFolderPath() const {
  return m_dvFolderPath;
}

void PlaybackReader::startPlayback() {
  if (running.load()) {
    qDebug() << "Playback already running, stopping previous playback";
    stopPlayback();
  }
  
  if (m_filePath.isEmpty()) {
    qDebug() << "No file selected for playback";
    return;
  }
  
  qDebug() << "Starting playback of file:" << m_filePath;
  running.store(true);

  // 重置帧率检测的静态变量，确保每次新播放都重新检测
  resetFrameRateDetection();

  // 检查是否有DV文件夹路径
  if (!m_dvFolderPath.isEmpty()) {
    // 如果有DV文件夹，启动带DV图像的播放
    m_playbackThread = std::thread(&PlaybackReader::playbackWithDVImages, this);
  } else {
    // 只支持RAW文件播放
    if (m_fileType == FileType::RAW) {
      m_playbackThread = std::thread(&PlaybackReader::playbackRAWFile, this);
    } else {
      qDebug() << "Unsupported file type, only RAW files are supported";
      running.store(false);
    }
  }
}

void PlaybackReader::stopPlayback() {
  if (!running.load()) {
    qDebug() << "Playback not running";
    return;
  }

  qDebug() << "Stopping playback";
  running.store(false);

  if (m_playbackThread.joinable()) {
    m_playbackThread.join();
  }

  qDebug() << "Playback stopped";
}

void PlaybackReader::startBatchPlayback(const QString &rootDir) {
  qDebug() << "Starting batch playback from root directory:" << rootDir;

  // 停止当前播放
  stopPlayback();
  stopBatchPlayback();

  // 收集所有会话文件夹
  m_sessionQueue = collectSessionFolders(rootDir);

  if (m_sessionQueue.isEmpty()) {
    qDebug() << "No valid sessions found in root directory:" << rootDir;
    emit batchPlaybackFinished();
    return;
  }

  qDebug() << "Found" << m_sessionQueue.size() << "sessions for batch playback";

  // 初始化批量播放状态
  m_currentSessionIndex = 0;
  m_batchPlaybackRunning.store(true);

  // 发出批量播放开始信号
  emit batchPlaybackStarted(m_sessionQueue.size());

  // 开始播放第一个会话
  playNextSession();
}

void PlaybackReader::stopBatchPlayback() {
  qDebug() << "Stopping batch playback";
  m_batchPlaybackRunning.store(false);

  // 停止当前播放
  stopPlayback();

  // 清空会话队列
  m_sessionQueue.clear();
  m_currentSessionIndex = 0;

  qDebug() << "Batch playback stopped";
}

bool PlaybackReader::isBatchPlaybackRunning() const {
  return m_batchPlaybackRunning.load();
}

void PlaybackReader::playbackRAWFile() {
  try {
    // 使用Metavision SDK打开RAW文件
    Metavision::Camera cam = Metavision::Camera::from_file(m_filePath.toStdString());
    
    // 获取相机几何信息
    int width = 1280;
    int height = 720;
    
    try {
      auto geometry = cam.geometry();
      width = geometry.width();
      height = geometry.height();
      qDebug() << "File geometry:" << width << "x" << height;
    } catch (const std::exception &e) {
      qDebug() << "Could not retrieve file geometry, using default:" << width << "x" << height;
    }
    
    // 设置累积周期（10ms）和帧率（30fps）
    const std::uint32_t accumulation_time_us = 10000;
    const double fps = 30.0;
    
    // 创建帧生成算法
    auto frame_gen = Metavision::PeriodicFrameGenerationAlgorithm(width, height, accumulation_time_us, fps);
    
    // 设置CD事件回调
    cam.cd().add_callback([&](const Metavision::EventCD *begin, const Metavision::EventCD *end) {
      // 事件处理和统计
      int event_count = std::distance(begin, end);
      if (event_count > 0 && running.load()) {
        m_totalEvents += event_count;
        
        // 将事件传递给帧生成算法
        frame_gen.process_events(begin, end);
      }
    });
    
    // 设置帧生成回调
    frame_gen.set_output_callback([this](Metavision::timestamp ts, cv::Mat &frame) {
      if (!running.load()) return;
      
      // 确保帧大小为1280x720 - 与原始相机保持一致
      cv::Mat resizedFrame;
      if (frame.cols != 1280 || frame.rows != 720) {
        cv::resize(frame, resizedFrame, cv::Size(1280, 720));
      } else {
        resizedFrame = frame;
      }
      
      // 转换为QImage，使用与DVSDataSource相同的图像格式
      // 确保颜色通道一致 - 如果是BGR需要转为RGB
      cv::Mat rgbFrame;
      if (resizedFrame.channels() == 3) {
        if (resizedFrame.type() == CV_8UC3) {
          // 如果已经是RGB格式
          rgbFrame = resizedFrame;
        } else {
          // 如果是BGR格式需要转换
          cv::cvtColor(resizedFrame, rgbFrame, cv::COLOR_BGR2RGB);
        }
      } else {
        // 如果是单通道图像，转换为三通道
        cv::cvtColor(resizedFrame, rgbFrame, cv::COLOR_GRAY2RGB);
      }
      
      // 创建与原始相机图像相同格式的QImage
      QImage qImg(rgbFrame.data, rgbFrame.cols, rgbFrame.rows, 
                rgbFrame.step, QImage::Format_RGB888);
      
      // 发出图像信号
      emit newFrame(qImg.copy());
      
      // 计算回放进度
      double progress = (double)ts / m_totalDuration;
      emit playbackProgress(std::min(progress, 1.0));
    });
    
    // 启动相机
    cam.start();
    
    // 播放循环
    while (running.load()) {
      // 使用Metavision的事件循环处理事件
      Metavision::EventLoop::poll_and_dispatch();
      
      // 降低CPU使用率
      QThread::msleep(5);
    }
    
    // 停止相机
    cam.stop();
    
  } catch (const std::exception &e) {
    qDebug() << "Error during RAW file playback:" << e.what();
  }
  
  handlePlaybackFinished();
  qDebug() << "RAW file playback finished";
}



void PlaybackReader::startPlayback(const PlaybackParams &params) {
  // 检查是否只是可视化处理
  if (params.visualizeOnly) {
    if (!params.rawFilePath.isEmpty()) {
      visualizeRawFile(params);
      return;
    }
  }
  
  // 设置回放模式
  dvsEnabled = params.dvsEnabled;
  dvEnabled = params.dvEnabled;
  
  qDebug() << "回放配置 - DVS回放:" << dvsEnabled << ", DV回放:" << dvEnabled;
  
  // 调用原始回放方法
  startPlayback(params.path);
}

void PlaybackReader::startPlayback(const QString &path) {
  loadFPN();
  QDir d(path);
  auto files = d.entryInfoList();
  QList<QFileInfo> dvImgs;
  QFileInfo dvsFileInfo;
  //筛选数据 - 只支持RAW文件
  for (auto file : files) {
    if (file.isFile()) {
      if ((file.fileName().endsWith(".jpg") ||
          file.fileName().endsWith(".bmp") ||
          file.fileName().endsWith(".png"))&& dvEnabled) {
        dvImgs.push_back(file);
        qDebug() << "dv imgs " << file.absoluteFilePath();
      } else if (file.fileName().endsWith(".raw") && dvsEnabled) {
        dvsFileInfo = file;
        qDebug() << "dvs raw file " << file.absoluteFilePath();
      }
    }
  }
  
  // 决定回放模式
  bool hasDVS = dvsEnabled && dvsFileInfo.exists();
  bool hasDV = dvEnabled && !dvImgs.empty();
  
  if (hasDVS && !hasDV) {
    // 只有DVS，没有DV - 只处理RAW文件
    if (dvsFileInfo.fileName().endsWith(".raw")) {
      PlaybackParams params;
      params.rawFilePath = dvsFileInfo.absoluteFilePath();
      params.visualizeOnly = true;
      visualizeRawFile(params);
      return;
    } else {
      qDebug() << "不支持的DVS文件格式，只支持RAW文件";
      emit complete();
      return;
    }
  }
  
  if (hasDV && !hasDVS) {
    // 只有DV，没有DVS
    // 对dv图片按时间排序
    std::sort(dvImgs.begin(), dvImgs.end(),
              [](const QFileInfo &a, const QFileInfo &b) {
                return a.baseName().toLongLong() < b.baseName().toLongLong();
              });
    
    playDVwithoutDVS(dvImgs);
    return;
  }
  
  if (hasDVS && hasDV) {
    // 同时有DVS和DV - 只处理RAW文件
    if (dvsFileInfo.fileName().endsWith(".raw")) {
      // 使用改进的同步播放方法
      playRAWWithDVImages(dvsFileInfo.absoluteFilePath(), dvImgs);
      return;
    } else {
      qDebug() << "不支持的DVS文件格式，只支持RAW文件";
      emit complete();
      return;
    }
  }
  
  // 没有可回放的内容
  qDebug() << "没有找到可回放的内容";
  emit complete();
}

// 仅回放DV图像的方法
void PlaybackReader::playDVwithoutDVS(QList<QFileInfo> dvImgs) {
  if (dvImgs.empty()) {
    qDebug() << "DV回放结束: 没有更多图片";
    emit complete();
    return;
  }

  // 在第一次调用时检测帧率 - 使用成员变量而不是静态变量
  static double detectedFPS = 0.0;
  static bool fpsDetected = false;
  static size_t lastImageCount = 0;

  // 如果图像数量变化，说明是新的播放会话，需要重新检测
  if (!fpsDetected || dvImgs.size() != lastImageCount) {
    detectedFPS = detectDVFrameRate(dvImgs);
    fpsDetected = true;
    lastImageCount = dvImgs.size();
    qDebug() << "DV纯回放模式 - 检测到的帧率:" << detectedFPS << "fps";
  }
  
  // 创建空白DVS图像，使用RGB格式以匹配真实DVS相机的输出
  QImage dvsImg(1280, 720, QImage::Format_RGB888);
  dvsImg.fill(Qt::black);
  
  // 获取第一张DV图像
  QFileInfo currentImgInfo = dvImgs.front();
  QImage dvImg = loadAndProcessDVImage(currentImgInfo.absoluteFilePath());
  
  // 从文件名中提取时间戳，处理_cropped后缀
  QString baseName = currentImgInfo.baseName();
  if (baseName.endsWith("_cropped")) {
    baseName = baseName.left(baseName.length() - 8);
  }
  auto lastTimestamp = baseName.toLongLong();
  qDebug() << "DV回放: 显示图片 " << currentImgInfo.fileName() 
           << " 时间戳: " << lastTimestamp;
  
  dvImgs.pop_front();
  
  // 发送图像对
  emit nextImagePair(dvImg, dvsImg);
  
  if (dvImgs.empty()) {
    qDebug() << "DV回放结束: 这是最后一张图片";
    handlePlaybackFinished();
    emit complete();
    return;
  }
  
  // 计算下一帧延迟 - 优化延迟计算
  QFileInfo nextImgInfo = dvImgs.front();
  QString nextBaseName = nextImgInfo.baseName();
  if (nextBaseName.endsWith("_cropped")) {
    nextBaseName = nextBaseName.left(nextBaseName.length() - 8);
  }
  auto nextTimestamp = nextBaseName.toLongLong();

  // 计算延迟：优先使用实际时间戳差异，但限制在合理范围内
  int timestampDelay = static_cast<int>((nextTimestamp - lastTimestamp) / 1000); // 转换为毫秒
  int fpsBasedDelay = static_cast<int>(1000.0 / detectedFPS); // 基于检测帧率的延迟

  // 选择合理的延迟：如果时间戳延迟过大或过小，使用基于帧率的延迟
  int delay;
  if (timestampDelay < 10 || timestampDelay > 200) {
    delay = fpsBasedDelay;
    qDebug() << "使用基于帧率的延迟:" << delay << "ms (时间戳延迟异常:" << timestampDelay << "ms)";
  } else {
    delay = timestampDelay;
  }

  // 确保最小延迟
  delay = std::max(delay, 16); // 最小16ms (约60fps)
  
  qDebug() << "DV回放: 下一帧延迟 " << delay << "ms, 下一帧: " 
           << nextImgInfo.fileName() << " 时间戳: " << nextTimestamp;
  
  // 使用QMetaObject::invokeMethod在主线程中调度下一帧
  QMetaObject::invokeMethod(this, [this, dvImgs, delay]() {
    QTimer::singleShot(delay, this, [this, dvImgs]() {
      playDVwithoutDVS(dvImgs);
    });
  }, Qt::QueuedConnection);
}



// 可视化RAW文件为图像序列
void PlaybackReader::visualizeRawFile(const PlaybackParams &params) {
  if (params.rawFilePath.isEmpty()) {
    qDebug() << "错误: 未提供RAW文件路径";
    emit complete();
    return;
  }
  
  // 直接回放RAW文件，而不是保存图片
  playRawFileDirectly(params.rawFilePath);
}



// 解析RAW文件头，提取元数据
bool PlaybackReader::parseRawFileHeader(FILE *file) {
  if (!file) {
    return false;
  }
  
  // 初始化默认值
  rawMetadata.width = 1280;
  rawMetadata.height = 720;
  rawMetadata.format = "EVT3";
  rawMetadata.valid = false;
  
  char line[1024];
  bool headerFound = false;
  bool headerEnd = false;
  
  // 回到文件开头
  fseek(file, 0, SEEK_SET);
  
  while (fgets(line, sizeof(line), file) != nullptr && !headerEnd) {
    std::string strLine(line);
    
    // 检查是否是头部行 (以% 开头)
    if (strLine.substr(0, 2) == "% ") {
      headerFound = true;
      
      // 移除前两个字符 "% "
      strLine = strLine.substr(2);
      
      // 检查是否是结束标记
      if (strLine.find("end") == 0) {
        headerEnd = true;
        rawMetadata.valid = true;
        continue;
      }
      
      // 解析键值对
      size_t spacePos = strLine.find(" ");
      if (spacePos != std::string::npos) {
        std::string key = strLine.substr(0, spacePos);
        std::string value = strLine.substr(spacePos + 1);
        
        // 移除尾部的换行符
        if (!value.empty() && value.back() == '\n') {
          value.pop_back();
        }
        
        qDebug() << "RAW头部:" << QString::fromStdString(key) << "=" << QString::fromStdString(value);
        
        // 提取宽度和高度
        if (key == "geometry") {
          std::regex geometryPattern("(\\d+)x(\\d+)");
          std::smatch matches;
          if (std::regex_search(value, matches, geometryPattern) && matches.size() > 2) {
            rawMetadata.width = std::stoi(matches[1].str());
            rawMetadata.height = std::stoi(matches[2].str());
            qDebug() << "  解析几何信息: 宽度=" << rawMetadata.width << " 高度=" << rawMetadata.height;
          }
        }
        // 提取格式信息
        else if (key == "format") {
          std::string formatStr = value;
          size_t formatEndPos = formatStr.find(";");
          if (formatEndPos != std::string::npos) {
            rawMetadata.format = QString::fromStdString(formatStr.substr(0, formatEndPos));
            qDebug() << "  解析格式: " << rawMetadata.format;
          }
        }
      }
    }
    else if (headerFound) {
      // 如果已经找到头部但当前行不是头部行，说明头部结束
      headerEnd = true;
      rawMetadata.valid = true;
      
      // 将文件指针回退到当前行，因为这一行应该是事件数据的开始
      fseek(file, -(long)strlen(line), SEEK_CUR);
      break;
    }
  }
  
  if (rawMetadata.valid) {
    qDebug() << "成功解析RAW文件头: 宽度=" << rawMetadata.width 
             << " 高度=" << rawMetadata.height 
             << " 格式=" << rawMetadata.format;
  } else {
    qDebug() << "RAW文件头解析失败或未找到";
  }
  
  return rawMetadata.valid;
}

// 解析EVT3事件数据
std::deque<PlaybackReader::Event> PlaybackReader::parseEvt3Events(
    const std::vector<uint8_t> &data, uint64_t beginTimestamp, uint64_t endTimestamp) {
  std::deque<Event> events;
  
  // EVT3格式中每个事件占用5个字节
  const size_t evt3_event_size = 5;
  const size_t num_events = data.size() / evt3_event_size;
  
  qDebug() << "解析EVT3事件: 数据大小=" << data.size() << " 预计事件数=" << num_events;
  
  for (size_t i = 0; i < num_events; i++) {
    const size_t offset = i * evt3_event_size;
    
    // 如果剩余数据不足5字节，跳过
    if (offset + 4 >= data.size()) {
      break;
    }
    
    // 解码事件
    uint8_t byte0 = data[offset];   // 低8位x坐标
    uint8_t byte1 = data[offset+1]; // 低7位y坐标 + 极性
    uint8_t byte2 = data[offset+2]; // y高3位 + x高2位 + 保留
    uint8_t byte3 = data[offset+3]; // 时间戳低8位
    uint8_t byte4 = data[offset+4]; // 时间戳高8位
    
    // 解析坐标和极性
    uint16_t x = byte0 | ((byte2 & 0x03) << 8);
    uint16_t y = ((byte1 & 0xFE) >> 1) | ((byte2 & 0x1C) << 6);
    uint8_t p = byte1 & 0x01;
    
    // 解析时间戳 (16位)
    uint16_t timestamp = byte3 | (byte4 << 8);
    
    // 创建事件
    Event e;
    e.x = x;
    e.y = y;
    e.g = p ? 1 : 0; // 将极性转换为当前代码使用的格式
    e.time = timestamp;
    
    // 只添加时间戳在指定范围内的事件
    if (e.time >= beginTimestamp && e.time <= endTimestamp) {
      events.push_back(e);
    }
  }
  
  qDebug() << "解析EVT3事件完成: 共解析" << events.size() << "个事件";
  return events;
}

// 直接回放RAW文件
void PlaybackReader::playRawFileDirectly(const QString &rawFilePath) {
  qDebug() << "开始直接回放RAW文件...";
  qDebug() << "输入文件: " << rawFilePath;
  
  // 加载FPN文件
  loadFPN();
  
  try {
    // 使用Metavision SDK打开RAW文件
    auto camera = Metavision::Camera::from_file(rawFilePath.toStdString());
    
    // 创建UI显示图像，使用RGB格式以匹配真实DVS相机的输出
    QImage dvsImage(1280, 720, QImage::Format_RGB888);
    dvsImage.fill(Qt::black);
    
    // 创建空白DV图像作为占位符
    QImage dvImage(1280, 720, QImage::Format_RGB888);
    dvImage.fill(Qt::black);
    
    // 创建帧生成算法 - 积累时间10000微秒，帧率25fps
    int width = 1280;
    int height = 720;
    const uint32_t accumulation_time = 10000;
    const double fps = 25.0;
    
    auto frame_generator = Metavision::PeriodicFrameGenerationAlgorithm(width, height, accumulation_time, fps);
    
    // 处理进度
    long total_events = 0;
    int frame_count = 0;
    bool is_first_frame = true;
    
    // 连接回调处理生成的帧
    frame_generator.set_output_callback([&](Metavision::timestamp ts, cv::Mat &frame) {
      // 转换为QImage并显示
      QImage frame_image = QImage(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888).copy();
      
      // 帧计数
      frame_count++;
      
      // 发送UI更新
      emit nextImagePair(dvImage, frame_image);
      
      // 显示进度
      if (frame_count % 10 == 0) {
        qDebug() << "已处理帧数: " << frame_count << " 累计事件: " << total_events;
        emit visualizationProgress(frame_count, 1000); // 假设总帧数约为1000
      }
      
      // 添加小延迟以避免UI过载
      QThread::msleep(40); // 约25fps
      
      // 如果是第一帧，初始化相机
      if (is_first_frame) {
        camera.start();
        is_first_frame = false;
      }
    });
    
    // 添加事件处理回调
    camera.cd().add_callback([&](const Metavision::EventCD *begin, const Metavision::EventCD *end) {
      // 将事件发送给帧生成器
      frame_generator.process_events(begin, end);
      
      // 统计事件
      total_events += (end - begin);
    });
    
    // 启动相机并等待处理完成
    qDebug() << "开始处理RAW文件事件...";
    camera.start();
    
    // 等待相机处理完成
    while (camera.is_running()) {
      static constexpr std::int64_t kSleepPeriodMs = 100;
      std::this_thread::sleep_for(std::chrono::milliseconds(kSleepPeriodMs));
    }
    
    // 处理完成
    qDebug() << "RAW文件回放完成, 共处理" << frame_count << "帧, " << total_events << "个事件";
    emit visualizationProgress(100, 100);
    handlePlaybackFinished();
    emit complete();
    
  } catch (const std::exception& e) {
    qDebug() << "RAW文件处理错误: " << e.what();
    
    // 尝试手动解析RAW文件
    qDebug() << "尝试手动解析RAW文件...";
    
    FILE *rawFile = fopen(rawFilePath.toStdString().c_str(), "rb");
    if (!rawFile) {
      qDebug() << "无法打开RAW文件: " << rawFilePath;
      handlePlaybackFinished();
      emit complete();
      return;
    }
    
    // 解析RAW文件头
    if (!parseRawFileHeader(rawFile)) {
      qDebug() << "无法解析RAW文件头，使用默认参数";
    }
    
    // 初始化图像，使用RGB格式以匹配真实DVS相机的输出
    QImage dvsImage(rawMetadata.width, rawMetadata.height, QImage::Format_RGB888);
    dvsImage.fill(Qt::black);
    
    // 创建空白DV图像作为占位符
    QImage dvImage(rawMetadata.width, rawMetadata.height, QImage::Format_RGB888);
    dvImage.fill(Qt::black);
    
    // 读取文件头之后的数据
    fseek(rawFile, 0, SEEK_END);
    long fileSize = ftell(rawFile);
    
    // 设置缓冲区大小
    const size_t buffer_size = 1024 * 1024; // 1MB缓冲区
    std::vector<uint8_t> buffer(buffer_size);
    
    // 回到数据起始位置
    fseek(rawFile, 0, SEEK_SET);
    
    // 跳过头部
    char line[1024];
    while (fgets(line, sizeof(line), rawFile) != nullptr) {
      std::string strLine(line);
      if (strLine.substr(0, 2) != "% ") {
        // 将文件指针回退到当前行，因为这一行应该是事件数据的开始
        fseek(rawFile, -(long)strlen(line), SEEK_CUR);
        break;
      }
    }
    
    long dataStartPos = ftell(rawFile);
    long dataSize = fileSize - dataStartPos;
    long processedBytes = 0;
    int frameCount = 0;
    uint64_t currentTimestamp = 0;
    
    qDebug() << "开始读取RAW文件数据: 总大小=" << dataSize << "字节";
    
    // 读取事件数据
    while (processedBytes < dataSize) {
      // 读取一块数据
      size_t bytesToRead = std::min(buffer_size, (size_t)(dataSize - processedBytes));
      size_t bytesRead = fread(buffer.data(), 1, bytesToRead, rawFile);
      
      if (bytesRead == 0) {
        break;
      }
      
      // 调整buffer大小以匹配实际读取的字节数
      buffer.resize(bytesRead);
      
      // 解析EVT3事件
      std::deque<Event> events = parseEvt3Events(buffer, 0, UINT64_MAX);
      
      // 绘制事件到图像
      for (const auto &event : events) {
        if (event.x >= 0 && event.x < rawMetadata.width && 
            event.y >= 0 && event.y < rawMetadata.height) {
          // 根据事件极性(g)设置像素颜色，正极性为白色，负极性为灰色
          // 现在使用RGB格式，需要设置RGB值
          QRgb pixelColor = (event.g == 1) ? qRgb(255, 255, 255) : qRgb(128, 128, 128);
          dvsImage.setPixel(event.x, event.y, pixelColor);
        }
        
        // 更新当前时间戳
        if (event.time > currentTimestamp) {
          currentTimestamp = event.time;
        }
      }
      
      // 更新处理进度
      processedBytes += bytesRead;
      int progress = static_cast<int>((processedBytes * 100) / dataSize);
      
      // 每处理一定量的数据发送一次图像更新
      if (frameCount % 5 == 0) {
        // 通知UI更新图像
        emit nextImagePair(dvImage, dvsImage);
        emit visualizationProgress(progress, 100);
        
        // 添加小延迟以允许UI更新
        QThread::msleep(40);
        
        // 清除图像准备下一帧
        dvsImage.fill(Qt::black);
      }
      
      frameCount++;
    }
    
    // 发送最后一帧
    emit nextImagePair(dvImage, dvsImage);
    emit visualizationProgress(100, 100);
    
    // 关闭文件
    fclose(rawFile);
    
    qDebug() << "RAW文件手动解析完成, 共处理" << frameCount << "个数据块";
    handlePlaybackFinished();
    emit complete();
  }
}





void PlaybackReader::stop() {}







void PlaybackReader::loadFPN() {
  using namespace std;
  FPN.resize(1280 * 720);
  QFile file(":/FPN_2.txt");
  file.open(QFile::ReadOnly);
  QTextStream ts(file.readAll());
  //    ifstream is("FPN_2.txt", ios_base::in);
  for (int i = 0; i < 1280 * 720; i++) {
    int index;
    ts >> index;
    FPN[i] = index;
    //        qDebug() << "fpn:" << index;
  }
}



// 实现兼容旧API的stopPlayback函数
void PlaybackReader::stopPlayback(bool saveYUVFrame) {
  qDebug() << "stopPlayback(bool) called with saveYUVFrame:" << saveYUVFrame;
  // 调用新的无参数版本
  stopPlayback();
  
  // 如果需要保存YUV帧，可以在这里添加相应的逻辑
  if (saveYUVFrame) {
    qDebug() << "YUV frame saving is not implemented in the new version";
  }
}

// 实现playbackWithDVImages方法
void PlaybackReader::playbackWithDVImages() {
  qDebug() << "Starting playback with DV images";
  
  // 加载DV图像
  QList<QFileInfo> dvImages = loadDVImages(m_dvFolderPath);
  if (dvImages.isEmpty()) {
    qDebug() << "No DV images found in folder:" << m_dvFolderPath;
  } else {
    qDebug() << "Loaded" << dvImages.size() << "DV images";
  }
  
  // 根据文件类型选择DVS播放方式
  if (m_fileType == FileType::RAW) {
    try {
      // 使用Metavision SDK打开RAW文件
      Metavision::Camera cam = Metavision::Camera::from_file(m_filePath.toStdString());
      
      // 获取相机几何信息
      int width = 1280;
      int height = 720;
      
      try {
        auto geometry = cam.geometry();
        width = geometry.width();
        height = geometry.height();
        qDebug() << "File geometry:" << width << "x" << height;
      } catch (const std::exception &e) {
        qDebug() << "Could not retrieve file geometry, using default:" << width << "x" << height;
      }
      
      // 设置累积周期（10ms）和帧率（30fps）
      const std::uint32_t accumulation_time_us = 10000;
      const double fps = 30.0;
      
      // 创建帧生成算法
      auto frame_gen = Metavision::PeriodicFrameGenerationAlgorithm(width, height, accumulation_time_us, fps);
      
      // 设置CD事件回调
      cam.cd().add_callback([&](const Metavision::EventCD *begin, const Metavision::EventCD *end) {
        // 事件处理和统计
        int event_count = std::distance(begin, end);
        if (event_count > 0 && running.load()) {
          m_totalEvents += event_count;
          
          // 将事件传递给帧生成算法
          frame_gen.process_events(begin, end);
        }
      });
      
      // 设置帧生成回调
      int dvImageIndex = 0;
      int totalDvImages = dvImages.size();
      
      frame_gen.set_output_callback([this, &dvImageIndex, totalDvImages, &dvImages](Metavision::timestamp ts, cv::Mat &frame) {
        if (!running.load()) return;
        
        // 确保帧大小为1280x720
        cv::Mat resizedFrame;
        if (frame.cols != 1280 || frame.rows != 720) {
          cv::resize(frame, resizedFrame, cv::Size(1280, 720));
        } else {
          resizedFrame = frame;
        }
        
        // 转换为RGB格式
        cv::Mat rgbFrame;
        if (resizedFrame.channels() == 3) {
          if (resizedFrame.type() == CV_8UC3) {
            rgbFrame = resizedFrame;
          } else {
            cv::cvtColor(resizedFrame, rgbFrame, cv::COLOR_BGR2RGB);
          }
        } else {
          cv::cvtColor(resizedFrame, rgbFrame, cv::COLOR_GRAY2RGB);
        }
        
        // 创建DVS图像 - 使用与原始相机相同格式
        QImage dvsImage(rgbFrame.data, rgbFrame.cols, rgbFrame.rows, 
                       rgbFrame.step, QImage::Format_RGB888);
        
        // 创建DV图像
        QImage dvImage;
        if (dvImageIndex < totalDvImages) {
          dvImage = loadAndProcessDVImage(dvImages[dvImageIndex].absoluteFilePath());
          dvImageIndex = (dvImageIndex + 1) % totalDvImages;  // 循环播放DV图像
        }
        
        // 发送图像对
        emit nextImagePair(dvImage, dvsImage.copy());
        
        // 单独发送DVS图像
        emit newFrame(dvsImage.copy());
        
        // 计算回放进度
        double progress = (double)ts / m_totalDuration;
        emit playbackProgress(std::min(progress, 1.0));
      });
      
      // 启动相机
      cam.start();
      
      // 播放循环
      while (running.load()) {
        // 使用Metavision的事件循环处理事件
        Metavision::EventLoop::poll_and_dispatch();
        
        // 降低CPU使用率
        QThread::msleep(5);
      }
      
      // 停止相机
      cam.stop();
      
    } catch (const std::exception &e) {
      qDebug() << "Error during RAW file playback with DV images:" << e.what();
    }
  }
  
  handlePlaybackFinished();
  emit complete();
  qDebug() << "Playback with DV images finished";
}

// 新的RAW文件与DV图像同步播放方法
void PlaybackReader::playRAWWithDVImages(const QString &rawFilePath, QList<QFileInfo> dvImgs) {
  qDebug() << "开始RAW文件与DV图像同步播放";
  qDebug() << "RAW文件:" << rawFilePath;
  qDebug() << "DV图像数量:" << dvImgs.size();

  if (dvImgs.isEmpty()) {
    qDebug() << "没有DV图像，使用纯RAW播放";
    PlaybackParams params;
    params.rawFilePath = rawFilePath;
    params.visualizeOnly = true;
    visualizeRawFile(params);
    return;
  }

  // 对DV图像按时间戳排序 - 处理_cropped后缀
  std::sort(dvImgs.begin(), dvImgs.end(),
            [](const QFileInfo &a, const QFileInfo &b) {
              // 提取时间戳，去除_cropped后缀
              QString baseNameA = a.baseName();
              QString baseNameB = b.baseName();

              // 移除_cropped后缀
              if (baseNameA.endsWith("_cropped")) {
                baseNameA = baseNameA.left(baseNameA.length() - 8);
              }
              if (baseNameB.endsWith("_cropped")) {
                baseNameB = baseNameB.left(baseNameB.length() - 8);
              }

              return baseNameA.toLongLong() < baseNameB.toLongLong();
            });

  // 获取DV图像的时间范围 - 处理_cropped后缀
  QString firstBaseName = dvImgs.first().baseName();
  QString lastBaseName = dvImgs.last().baseName();

  // 移除_cropped后缀
  if (firstBaseName.endsWith("_cropped")) {
    firstBaseName = firstBaseName.left(firstBaseName.length() - 8);
  }
  if (lastBaseName.endsWith("_cropped")) {
    lastBaseName = lastBaseName.left(lastBaseName.length() - 8);
  }

  uint64_t firstDVTimestamp = firstBaseName.toLongLong();
  uint64_t lastDVTimestamp = lastBaseName.toLongLong();
  uint64_t dvDuration = lastDVTimestamp - firstDVTimestamp;

  qDebug() << "DV时间范围: " << firstDVTimestamp << " - " << lastDVTimestamp;
  qDebug() << "DV总时长: " << dvDuration << " 微秒 (" << dvDuration/1000000.0 << " 秒)";

  // 智能帧率检测 - 分析DV图像时间戳分布
  double detectedFPS = detectDVFrameRate(dvImgs);
  qDebug() << "检测到的DV帧率: " << detectedFPS << " fps";

  // 时间同步变量 - 在try块外声明
  int dvImageIndex = 0;
  QImage currentDVImage;
  uint64_t dvsStartTimestamp = 0;
  uint64_t dvsEndTimestamp = 0;
  bool timestampMappingEstablished = false;
  double timeScaleFactor = 1.0;

  try {
    // 使用Metavision SDK打开RAW文件
    auto camera = Metavision::Camera::from_file(rawFilePath.toStdString());

    // 获取相机几何信息
    int width = 1280;
    int height = 720;

    try {
      auto geometry = camera.geometry();
      width = geometry.width();
      height = geometry.height();
      qDebug() << "RAW文件几何信息:" << width << "x" << height;
    } catch (const std::exception &e) {
      qDebug() << "无法获取RAW文件几何信息，使用默认值:" << width << "x" << height;
    }

    // 尝试获取RAW文件的时间范围
    try {
      // 预扫描文件获取时间范围
      auto tempCamera = Metavision::Camera::from_file(rawFilePath.toStdString());
      uint64_t firstEventTime = 0;
      uint64_t lastEventTime = 0;
      bool firstEventFound = false;

      tempCamera.cd().add_callback([&](const Metavision::EventCD *begin, const Metavision::EventCD *end) {
        if (begin < end) {
          if (!firstEventFound) {
            firstEventTime = begin->t;
            firstEventFound = true;
          }
          lastEventTime = (end - 1)->t;
        }
      });

      tempCamera.start();
      while (tempCamera.is_running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
      tempCamera.stop();

      if (firstEventFound) {
        dvsStartTimestamp = firstEventTime;
        dvsEndTimestamp = lastEventTime;
        uint64_t dvsTotalDuration = dvsEndTimestamp - dvsStartTimestamp;

        if (dvsTotalDuration > 0 && dvDuration > 0) {
          timeScaleFactor = (double)dvDuration / dvsTotalDuration;

          // 验证时间缩放因子的合理性
          double impliedDVSFPS = 1000000.0 / (dvsTotalDuration / (double)dvImgs.size());
          double fpsRatio = detectedFPS / impliedDVSFPS;

          qDebug() << "时间同步分析:";
          qDebug() << "  - DVS时间范围:" << dvsStartTimestamp << "-" << dvsEndTimestamp;
          qDebug() << "  - DVS总时长:" << dvsTotalDuration << "微秒";
          qDebug() << "  - DV检测帧率:" << detectedFPS << "fps";
          qDebug() << "  - DVS隐含帧率:" << impliedDVSFPS << "fps";
          qDebug() << "  - 帧率比值:" << fpsRatio;
          qDebug() << "  - 时间缩放因子:" << timeScaleFactor;
        }
      }
    } catch (const std::exception &e) {
      qDebug() << "预扫描RAW文件失败，将在播放时动态计算:" << e.what();
    }

    // 创建帧生成算法 - 使用检测到的DV帧率和配置文件中的积累时间
    const uint32_t accumulation_time = getAccumulationTimeFromConfig();
    const double fps = detectedFPS; // 使用检测到的帧率

    auto frame_generator = Metavision::PeriodicFrameGenerationAlgorithm(width, height, accumulation_time, fps);

    qDebug() << "DVS帧生成参数 - 帧率:" << fps << "fps, 积累时间:" << accumulation_time << "微秒 (来源:配置文件)";

    // 初始化第一张DV图像
    if (dvImageIndex < dvImgs.size()) {
      currentDVImage = loadAndProcessDVImage(dvImgs[dvImageIndex].absoluteFilePath());
      qDebug() << "初始DV图像:" << dvImgs[dvImageIndex].fileName();
    }

    // 设置帧生成回调 - 实现精确时间同步
    frame_generator.set_output_callback([&](Metavision::timestamp ts, cv::Mat &frame) {
      if (!running.load()) return;

      // DVS时间戳（微秒）
      uint64_t dvsTimestamp = static_cast<uint64_t>(ts);

      // 建立时间戳映射关系（仅在第一帧时）
      if (!timestampMappingEstablished) {
        // 如果预扫描没有成功，使用当前时间戳作为起始点
        if (dvsStartTimestamp == 0) {
          dvsStartTimestamp = dvsTimestamp;

          // 如果没有预扫描数据，基于检测到的帧率估算时间缩放因子
          if (timeScaleFactor == 1.0 && dvDuration > 0) {
            // 估算DVS总时长：假设DVS事件覆盖整个DV录制期间
            double estimatedDVSFPS = detectedFPS; // 假设DVS和DV同步录制
            uint64_t estimatedDVSTotalDuration = dvDuration;
            timeScaleFactor = (double)dvDuration / estimatedDVSTotalDuration;

            qDebug() << "基于检测帧率估算时间缩放因子:" << timeScaleFactor;
          }
        }
        timestampMappingEstablished = true;

        qDebug() << "时间戳映射建立完成:";
        qDebug() << "  - DVS起始时间戳:" << dvsStartTimestamp;
        qDebug() << "  - DV起始时间戳:" << firstDVTimestamp;
        qDebug() << "  - 最终时间缩放因子:" << timeScaleFactor;
        qDebug() << "  - 检测到的DV帧率:" << detectedFPS << "fps";
      }

      // 计算当前DVS相对时间
      uint64_t dvsRelativeTime = dvsTimestamp - dvsStartTimestamp;

      // 应用时间缩放因子，将DVS时间映射到DV时间范围
      uint64_t scaledDVSTime = (uint64_t)(dvsRelativeTime * timeScaleFactor);
      uint64_t mappedDVTimestamp = firstDVTimestamp + scaledDVSTime;

      // 根据映射的时间戳找到对应的DV图像 - 优化搜索算法
      int targetDVIndex = -1;
      uint64_t minTimeDiff = UINT64_MAX;

      // 基于检测到的帧率计算搜索窗口
      uint64_t frameInterval = static_cast<uint64_t>(1000000.0 / detectedFPS); // 微秒
      uint64_t searchWindow = frameInterval / 2; // 搜索窗口为半个帧间隔

      // 优化搜索：从当前索引附近开始搜索，减少计算量
      int searchStart = std::max(0, dvImageIndex - 5);
      int searchEnd = std::min(dvImgs.size(), dvImageIndex + 10);

      for (int i = searchStart; i < searchEnd; i++) {
        // 提取时间戳，处理_cropped后缀
        QString baseName = dvImgs[i].baseName();
        if (baseName.endsWith("_cropped")) {
          baseName = baseName.left(baseName.length() - 8);
        }

        uint64_t dvTimestamp = baseName.toLongLong();
        uint64_t timeDiff = (mappedDVTimestamp > dvTimestamp) ?
                           (mappedDVTimestamp - dvTimestamp) :
                           (dvTimestamp - mappedDVTimestamp);

        // 只考虑在搜索窗口内的图像
        if (timeDiff <= searchWindow && timeDiff < minTimeDiff) {
          minTimeDiff = timeDiff;
          targetDVIndex = i;
        }
      }

      // 如果在窗口内没找到合适的图像，扩大搜索范围
      if (targetDVIndex == -1) {
        for (int i = 0; i < dvImgs.size(); i++) {
          QString baseName = dvImgs[i].baseName();
          if (baseName.endsWith("_cropped")) {
            baseName = baseName.left(baseName.length() - 8);
          }

          uint64_t dvTimestamp = baseName.toLongLong();
          uint64_t timeDiff = (mappedDVTimestamp > dvTimestamp) ?
                             (mappedDVTimestamp - dvTimestamp) :
                             (dvTimestamp - mappedDVTimestamp);

          if (timeDiff < minTimeDiff) {
            minTimeDiff = timeDiff;
            targetDVIndex = i;
          }
        }
      }

      // 更新DV图像（如果找到更匹配的）
      if (targetDVIndex >= 0 && targetDVIndex != dvImageIndex) {
        dvImageIndex = targetDVIndex;
        currentDVImage = loadAndProcessDVImage(dvImgs[dvImageIndex].absoluteFilePath());

        static int logCount = 0;
        if (logCount++ % 30 == 0) { // 每30帧打印一次日志
          QString actualBaseName = dvImgs[dvImageIndex].baseName();
          if (actualBaseName.endsWith("_cropped")) {
            actualBaseName = actualBaseName.left(actualBaseName.length() - 8);
          }

          uint64_t actualDVTimestamp = actualBaseName.toLongLong();
          double syncAccuracy = (double)minTimeDiff / (1000000.0 / detectedFPS) * 100.0; // 同步精度百分比

          qDebug() << "帧率同步状态:";
          qDebug() << "  - DVS时间:" << dvsTimestamp << "微秒";
          qDebug() << "  - 映射DV时间:" << mappedDVTimestamp << "微秒";
          qDebug() << "  - 实际DV时间:" << actualDVTimestamp << "微秒";
          qDebug() << "  - 时间差:" << minTimeDiff << "微秒";
          qDebug() << "  - 同步精度:" << QString::number(syncAccuracy, 'f', 1) << "%";
          qDebug() << "  - 当前帧率:" << detectedFPS << "fps";
        }
      }

      // 确保DVS帧大小正确
      cv::Mat resizedFrame;
      if (frame.cols != 1280 || frame.rows != 720) {
        cv::resize(frame, resizedFrame, cv::Size(1280, 720));
      } else {
        resizedFrame = frame;
      }

      // 转换DVS帧为RGB格式
      cv::Mat rgbFrame;
      if (resizedFrame.channels() == 3) {
        rgbFrame = resizedFrame;
      } else {
        cv::cvtColor(resizedFrame, rgbFrame, cv::COLOR_GRAY2RGB);
      }

      // 创建DVS图像
      QImage dvsImage(rgbFrame.data, rgbFrame.cols, rgbFrame.rows,
                     rgbFrame.step, QImage::Format_RGB888);

      // 发送同步的图像对
      emit nextImagePair(currentDVImage, dvsImage.copy());

      // 计算并发送进度（基于DV图像总数）
      double progress = (double)(dvImageIndex + 1) / dvImgs.size();
      emit playbackProgress(std::min(progress, 1.0));
    });

    // 添加事件处理回调 - 增加时间范围控制
    camera.cd().add_callback([&](const Metavision::EventCD *begin, const Metavision::EventCD *end) {
      if (!running.load()) return;

      // 如果还没有建立时间戳映射，记录DVS结束时间戳
      if (end > begin) {
        dvsEndTimestamp = (end - 1)->t; // 获取最后一个事件的时间戳
      }

      frame_generator.process_events(begin, end);
      m_totalEvents += (end - begin);
    });

    // 启动相机并播放
    qDebug() << "开始同步播放RAW文件和DV图像";
    camera.start();

    // 播放循环 - 增加超时保护
    auto startTime = std::chrono::steady_clock::now();
    const auto maxPlaybackTime = std::chrono::seconds(300); // 5分钟超时保护

    while (running.load() && camera.is_running()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));

      // 检查超时
      auto currentTime = std::chrono::steady_clock::now();
      if (currentTime - startTime > maxPlaybackTime) {
        qDebug() << "播放超时，强制停止";
        break;
      }
    }

    camera.stop();
    qDebug() << "RAW文件与DV图像同步播放完成";

  } catch (const std::exception& e) {
    qDebug() << "RAW文件同步播放错误:" << e.what();
  }

  // 确保播放状态正确设置
  running.store(false);

  handlePlaybackFinished();
  emit complete();
}

// 加载DV文件夹中的图像
QList<QFileInfo> PlaybackReader::loadDVImages(const QString &folderPath) {
  QList<QFileInfo> imageFiles;
  
  if (folderPath.isEmpty()) {
    qDebug() << "DV folder path is empty";
    return imageFiles;
  }
  
  QDir dir(folderPath);
  if (!dir.exists()) {
    qDebug() << "DV folder does not exist:" << folderPath;
    return imageFiles;
  }
  
  // 设置过滤器，只获取图像文件
  QStringList filters;
  filters << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp";
  dir.setNameFilters(filters);
  dir.setSorting(QDir::Name); // 按文件名排序
  
  // 获取所有图像文件
  QFileInfoList fileList = dir.entryInfoList();
  
  qDebug() << "Found" << fileList.size() << "image files in DV folder";

  return fileList;
}

// 智能帧率检测方法 - 分析DV图像时间戳分布
double PlaybackReader::detectDVFrameRate(const QList<QFileInfo> &dvImgs) {
  if (dvImgs.size() < 2) {
    qDebug() << "DV图像数量不足，使用默认帧率30fps";
    return 30.0;
  }

  // 提取所有时间戳
  QList<uint64_t> timestamps;
  for (const auto &fileInfo : dvImgs) {
    QString baseName = fileInfo.baseName();

    // 移除_cropped后缀
    if (baseName.endsWith("_cropped")) {
      baseName = baseName.left(baseName.length() - 8);
    }

    bool ok;
    uint64_t timestamp = baseName.toLongLong(&ok);
    if (ok) {
      timestamps.append(timestamp);
    }
  }

  if (timestamps.size() < 2) {
    qDebug() << "有效时间戳数量不足，使用默认帧率30fps";
    return 30.0;
  }

  // 按时间戳排序
  std::sort(timestamps.begin(), timestamps.end());

  // 计算时间间隔
  QList<uint64_t> intervals;
  for (int i = 1; i < timestamps.size(); i++) {
    uint64_t interval = timestamps[i] - timestamps[i-1];
    if (interval > 0 && interval < 1000000) { // 过滤异常间隔（小于1秒）
      intervals.append(interval);
    }
  }

  if (intervals.isEmpty()) {
    qDebug() << "无有效时间间隔，使用默认帧率30fps";
    return 30.0;
  }

  // 计算平均时间间隔（微秒）
  uint64_t totalInterval = 0;
  for (uint64_t interval : intervals) {
    totalInterval += interval;
  }
  double avgInterval = (double)totalInterval / intervals.size();

  // 转换为帧率（fps）
  double detectedFPS = 1000000.0 / avgInterval; // 1秒 = 1,000,000微秒

  // 限制帧率范围在合理区间内
  if (detectedFPS < 1.0) {
    qDebug() << "检测到的帧率过低(" << detectedFPS << ")，使用最小值1fps";
    detectedFPS = 1.0;
  } else if (detectedFPS > 120.0) {
    qDebug() << "检测到的帧率过高(" << detectedFPS << ")，使用最大值120fps";
    detectedFPS = 120.0;
  }

  qDebug() << "帧率检测详情:";
  qDebug() << "  - 总图像数:" << dvImgs.size();
  qDebug() << "  - 有效时间戳数:" << timestamps.size();
  qDebug() << "  - 有效间隔数:" << intervals.size();
  qDebug() << "  - 平均间隔:" << avgInterval << "微秒";
  qDebug() << "  - 检测帧率:" << detectedFPS << "fps";

  return detectedFPS;
}

// 计算最优积累时间 - 基于检测到的帧率
uint32_t PlaybackReader::calculateOptimalAccumulationTime(double fps) {
  // 积累时间应该略小于帧间隔，以确保事件能及时处理
  // 帧间隔 = 1/fps 秒 = 1000000/fps 微秒
  double frameInterval = 1000000.0 / fps; // 微秒

  // 积累时间设为帧间隔的80%，确保有足够的事件积累但不会延迟太久
  uint32_t accumulationTime = static_cast<uint32_t>(frameInterval * 0.8);

  // 限制积累时间在合理范围内
  const uint32_t minAccTime = 5000;   // 最小5ms
  const uint32_t maxAccTime = 50000;  // 最大50ms

  if (accumulationTime < minAccTime) {
    accumulationTime = minAccTime;
  } else if (accumulationTime > maxAccTime) {
    accumulationTime = maxAccTime;
  }

  qDebug() << "积累时间计算:";
  qDebug() << "  - 帧率:" << fps << "fps";
  qDebug() << "  - 帧间隔:" << frameInterval << "微秒";
  qDebug() << "  - 积累时间:" << accumulationTime << "微秒";

  return accumulationTime;
}

// 重置帧率检测状态 - 确保每次新播放都重新检测
void PlaybackReader::resetFrameRateDetection() {
  // 这个方法用于重置playDVwithoutDVS中的静态变量
  // 由于C++的限制，我们无法直接重置函数内的静态变量
  // 但我们可以通过其他方式来标记需要重新检测
  qDebug() << "重置帧率检测状态，下次播放将重新检测帧率";
}

// 从配置文件读取积累时间 - 线程安全的配置读取
uint32_t PlaybackReader::getAccumulationTimeFromConfig() {
  uint32_t accumulationTime = 10000; // 默认10ms (10000微秒)

  try {
    // 使用ConfigManager获取配置文件路径，确保线程安全
    QString configPath = ConfigManager::getInstance().getConfigPath();
    QFile configFile(configPath);

    if (!configFile.open(QIODevice::ReadOnly)) {
      qDebug() << "无法打开配置文件，使用默认积累时间:" << accumulationTime << "微秒";
      return accumulationTime;
    }

    QByteArray jsonData = configFile.readAll();
    configFile.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (!doc.isObject()) {
      qDebug() << "配置文件格式错误，使用默认积累时间:" << accumulationTime << "微秒";
      return accumulationTime;
    }

    QJsonObject rootObj = doc.object();
    if (rootObj.contains("dvs") && rootObj["dvs"].isObject()) {
      QJsonObject dvsObj = rootObj["dvs"].toObject();

      // 读取eventFrameTime参数（单位：毫秒）
      if (dvsObj.contains("eventFrameTime") && dvsObj["eventFrameTime"].isDouble()) {
        int eventFrameTimeMs = dvsObj["eventFrameTime"].toInt();
        accumulationTime = eventFrameTimeMs * 1000; // 转换为微秒

        qDebug() << "从配置文件读取DVS积累时间:" << eventFrameTimeMs << "ms (" << accumulationTime << "微秒)";
      } else {
        qDebug() << "配置文件中未找到dvs.eventFrameTime，使用默认值:" << accumulationTime << "微秒";
      }
    } else {
      qDebug() << "配置文件中未找到dvs配置节，使用默认积累时间:" << accumulationTime << "微秒";
    }

  } catch (const std::exception &e) {
    qDebug() << "读取配置文件时发生异常，使用默认积累时间:" << e.what();
  }

  // 限制积累时间在合理范围内
  const uint32_t minAccTime = 5000;   // 最小5ms
  const uint32_t maxAccTime = 50000;  // 最大50ms

  if (accumulationTime < minAccTime) {
    qDebug() << "积累时间过小，调整为最小值:" << minAccTime << "微秒";
    accumulationTime = minAccTime;
  } else if (accumulationTime > maxAccTime) {
    qDebug() << "积累时间过大，调整为最大值:" << maxAccTime << "微秒";
    accumulationTime = maxAccTime;
  }

  return accumulationTime;
}

// 加载并处理DV图像，应用与海康相机相同的颜色处理
QImage PlaybackReader::loadAndProcessDVImage(const QString &imagePath) {
  // 加载原始图像
  QImage originalImg(imagePath);

  if (originalImg.isNull()) {
    qDebug() << "无法加载DV图像:" << imagePath;
    return QImage();
  }

  // 确保图像格式为RGB888，模仿海康相机的输出格式
  if (originalImg.format() != QImage::Format_RGB888) {
    originalImg = originalImg.convertToFormat(QImage::Format_RGB888);
  }

  // 注意：这里不应用海康相机的颜色校正，因为DV图像已经是处理过的图像文件
  // 海康相机的颜色校正是针对Bayer格式的原始数据，而DV图像文件已经是RGB格式

  qDebug() << "成功加载DV图像:" << imagePath << "尺寸:" << originalImg.size();
  return originalImg;
}

QList<PlaybackReader::SessionInfo> PlaybackReader::collectSessionFolders(const QString &rootDir) {
  QList<SessionInfo> sessionList;
  QDir root(rootDir);

  if (!root.exists()) {
    qDebug() << "Root directory does not exist:" << rootDir;
    return sessionList;
  }

  // 获取所有子目录
  QFileInfoList subDirs = root.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

  for (const QFileInfo& subDir : subDirs) {
    QDir sessionDir(subDir.absoluteFilePath());

    // 修改目录结构检查：DV播放dv_cropped文件夹，DVS播放dvs_raw源文件
    bool hasDV = sessionDir.exists("dv_cropped");
    bool hasDVS = sessionDir.exists("dvs_raw");

    if (hasDV && hasDVS) {
      SessionInfo session;
      session.sessionName = subDir.fileName();
      session.dvPath = sessionDir.absoluteFilePath("dv_cropped");
      session.dvsPath = sessionDir.absoluteFilePath("dvs_raw");

      sessionList.append(session);
      qDebug() << "Found valid session:" << session.sessionName
               << "DV:" << session.dvPath
               << "DVS:" << session.dvsPath;
    } else {
      qDebug() << "Skipping invalid session:" << subDir.fileName()
               << "hasDV:" << hasDV << "hasDVS:" << hasDVS;
    }

    // 递归搜索子目录
    auto subSessions = collectSessionFolders(subDir.absoluteFilePath());
    sessionList.append(subSessions);
  }

  return sessionList;
}

void PlaybackReader::playNextSession() {
  if (!m_batchPlaybackRunning.load()) {
    qDebug() << "Batch playback stopped, not playing next session";
    return;
  }

  if (m_currentSessionIndex >= m_sessionQueue.size()) {
    qDebug() << "All sessions completed, finishing batch playback";
    m_batchPlaybackRunning.store(false);
    emit batchPlaybackFinished();
    return;
  }

  const SessionInfo& session = m_sessionQueue[m_currentSessionIndex];
  qDebug() << "Playing session" << (m_currentSessionIndex + 1) << "of" << m_sessionQueue.size()
           << ":" << session.sessionName;

  // 发出进度信号
  emit batchPlaybackProgress(m_currentSessionIndex + 1, m_sessionQueue.size(), session.sessionName);

  // 查找DVS文件
  QString dvsFile = findDVSFile(session.dvsPath);
  if (dvsFile.isEmpty()) {
    qDebug() << "No valid DVS file found in:" << session.dvsPath;
    // 跳到下一个会话
    m_currentSessionIndex++;
    QMetaObject::invokeMethod(this, [this]() {
      QTimer::singleShot(100, this, &PlaybackReader::playNextSession);
    }, Qt::QueuedConnection);
    return;
  }

  // 加载DV图像
  QList<QFileInfo> dvImages = loadDVImages(session.dvPath);
  if (dvImages.isEmpty()) {
    qDebug() << "No DV images found in:" << session.dvPath;
    // 跳到下一个会话
    m_currentSessionIndex++;
    QMetaObject::invokeMethod(this, [this]() {
      QTimer::singleShot(100, this, &PlaybackReader::playNextSession);
    }, Qt::QueuedConnection);
    return;
  }

  qDebug() << "Found" << dvImages.size() << "DV images and DVS file:" << dvsFile;

  // 确保停止当前播放
  stopPlayback();

  // 等待当前播放完全停止
  if (m_playbackThread.joinable()) {
    m_playbackThread.join();
  }

  // 设置新的播放参数
  setFilePath(dvsFile);
  setDVFolderPath(session.dvPath);

  // 启动同步播放
  running.store(true);
  m_playbackThread = std::thread(&PlaybackReader::playRAWWithDVImages, this, dvsFile, dvImages);

  // 不在这里增加索引，而是在播放完成后增加
  // m_currentSessionIndex++; // 移除这行
}

QString PlaybackReader::findDVSFile(const QString &dvsPath) {
  QDir dvsDir(dvsPath);
  if (!dvsDir.exists()) {
    qDebug() << "DVS directory does not exist:" << dvsPath;
    return QString();
  }

  // 只查找.raw文件，删除对.bin文件的支持
  QStringList filters;
  filters << "*.raw";
  dvsDir.setNameFilters(filters);

  QFileInfoList dvsFiles = dvsDir.entryInfoList(QDir::Files);

  if (dvsFiles.isEmpty()) {
    qDebug() << "No RAW files found in:" << dvsPath;
    return QString();
  }

  // 返回第一个找到的RAW文件
  QString dvsFile = dvsFiles.first().absoluteFilePath();
  qDebug() << "Found DVS RAW file:" << dvsFile;
  return dvsFile;
}

void PlaybackReader::handlePlaybackFinished() {
  qDebug() << "Playback finished, cleaning up...";

  // 确保播放线程完全停止
  running.store(false);

  // 发出常规的播放完成信号
  emit playbackFinished();

  // 如果是批量播放模式，自动播放下一个会话
  if (m_batchPlaybackRunning.load()) {
    qDebug() << "Current session completed, preparing next session";

    // 增加会话索引，指向下一个会话
    m_currentSessionIndex++;
    qDebug() << "单个会话播放完成，当前索引:" << m_currentSessionIndex << "总会话数:" << m_sessionQueue.size();

    // 检查是否还有更多会话
    if (m_currentSessionIndex < m_sessionQueue.size()) {
      qDebug() << "Scheduling next session in 1000ms to ensure cleanup";
      // 给足够的时间让当前会话完全清理，使用QMetaObject::invokeMethod确保在主线程中执行
      QMetaObject::invokeMethod(this, [this]() {
        QTimer::singleShot(1000, this, &PlaybackReader::playNextSession);
      }, Qt::QueuedConnection);
    } else {
      qDebug() << "All sessions completed";
      m_batchPlaybackRunning.store(false);
      emit batchPlaybackFinished();
    }
  }
}
