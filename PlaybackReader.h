//
// Created by pe on 2021/4/22.
//

#pragma once

#include <QByteArray>
#include <QFileInfo>
#include <QImage>
#include <QObject>
#include <QTime>
#include <atomic>
#include <deque>
#include <memory>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>

// 添加Metavision SDK的相关头文件
#include <metavision/sdk/driver/camera.h>
#include <metavision/sdk/base/events/event_cd.h>
#include <metavision/sdk/core/algorithms/periodic_frame_generation_algorithm.h>

class PlaybackReader : public QObject {
  Q_OBJECT
public:
  // 文件类型枚举
  enum class FileType {
    RAW,    // Metavision RAW格式
    UNKNOWN // 未知格式
  };

  PlaybackReader();
  ~PlaybackReader();

  // 文件路径相关方法
  void setFilePath(const QString &filePath);
  QString getFilePath() const;
  
  // 设置DV文件夹路径
  void setDVFolderPath(const QString &folderPath);
  QString getDVFolderPath() const;

  // 简化的播放控制方法
  void startPlayback();
  void stopPlayback();

  // 批量播放相关方法
  void startBatchPlayback(const QString &rootDir);
  void stopBatchPlayback();
  bool isBatchPlaybackRunning() const;

  // 回放参数结构体
  struct PlaybackParams {
    QString path;
    bool dvsEnabled = true;
    bool dvEnabled = true;
    uint32_t speed = 1;
    
    // 用于RAW文件可视化的参数
    QString rawFilePath; // RAW文件路径
    bool visualizeOnly = false;
    
    // DV文件夹路径
    QString dvFolderPath;
  };

  // 可视化RAW文件为图像序列
  void visualizeRawFile(const PlaybackParams &params);

signals:
  void newFrame(QImage frame);
  void playbackProgress(double progress); // 0.0 - 1.0
  void playbackFinished();

  // 批量播放相关信号
  void batchPlaybackStarted(int totalSessions);
  void batchPlaybackProgress(int currentSession, int totalSessions, const QString &currentSessionName);
  void batchPlaybackFinished();

private:
  // 文件播放方法
  void playbackRAWFile(); // 播放RAW文件
  void playbackWithDVImages(); // 同时播放DVS和DV图像

  // 成员变量
  QString m_filePath;
  QString m_dvFolderPath; // DV文件夹路径
  FileType m_fileType;
  std::thread m_playbackThread;
  std::atomic<bool> running{false};

  // 批量播放相关成员变量
  struct SessionInfo {
    QString sessionName;
    QString dvPath;
    QString dvsPath;
  };
  QList<SessionInfo> m_sessionQueue;
  int m_currentSessionIndex = 0;
  std::atomic<bool> m_batchPlaybackRunning{false};

  // 递归收集所有子会话路径
  QList<SessionInfo> collectSessionFolders(const QString &rootDir);

  // 播放下一个会话
  void playNextSession();

  // 查找DVS文件（支持.raw和.bin）
  QString findDVSFile(const QString &dvsPath);

  // 处理播放完成事件
  void handlePlaybackFinished();
  
  // 播放统计信息
  long long m_totalEvents{0};
  long long m_totalDuration{0}; // 微秒
  
  // 加载DV文件夹中的图像
  QList<QFileInfo> loadDVImages(const QString &folderPath);

  // 加载并处理DV图像，应用与海康相机相同的颜色处理
  QImage loadAndProcessDVImage(const QString &imagePath);

  // Event结构体定义（用于RAW文件解析）
  struct Event {
    int x, y;
    int g;
    uint64_t time;
  };

  void loadFPN();

  std::vector<int> FPN;

  void playDVwithoutDVS(QList<QFileInfo> dvImgs);
  

  
  // 直接回放RAW文件
  void playRawFileDirectly(const QString &rawFilePath);

  // RAW文件与DV图像同步播放
  void playRAWWithDVImages(const QString &rawFilePath, QList<QFileInfo> dvImgs);

  // 智能帧率检测方法
  double detectDVFrameRate(const QList<QFileInfo> &dvImgs);

  // 计算最优积累时间
  uint32_t calculateOptimalAccumulationTime(double fps);

  // 重置帧率检测状态
  void resetFrameRateDetection();

  // 从配置文件读取积累时间
  uint32_t getAccumulationTimeFromConfig();

  // 解析RAW文件头
  bool parseRawFileHeader(FILE *file);
  
  // 解析EVT3事件格式
  std::deque<Event> parseEvt3Events(const std::vector<uint8_t> &data, uint64_t beginTimestamp, uint64_t endTimestamp);

  // 当前回放配置
  bool dvEnabled = true;
  bool dvsEnabled = true;
  
  // 用于记录RAW文件元数据
  struct RawFileMetadata {
    int width;
    int height;
    QString format;
    bool valid;
  } rawMetadata;

private slots:

public slots:

  void startPlayback(const QString &path);
  void startPlayback(const PlaybackParams &params);
  
  // 兼容旧API
  void stopPlayback(bool saveYUVFrame);

  void stop();

signals:

  void nextImagePair(QImage dv, QImage dvs);
  void nextImagePairWithFilenames(QImage dv, QImage dvs, QString dvFilename, QString dvsFilename);

  void complete();
  
  // 可视化进度信号
  void visualizationProgress(int current, int total);
  void visualizationComplete(const QString &outputDir);

public:
  static PlaybackReader *getInstance() {
    static PlaybackReader instance;
    return &instance;
  }
};
