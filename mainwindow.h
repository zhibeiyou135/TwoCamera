#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QPushButton>
#include <QProgressBar>
#include <QCheckBox>
#include <QLineEdit>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <thread>
#include "socket/modbussocket.h"
#include "camera/CropConfig.h"

// 前向声明检测相关类
class DetectModule_DV;
class DetectModule_DVS;
class DualCameraResultManager;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  MainWindow(QWidget *parent = nullptr);

  ~MainWindow();
  
  // 设置是否启用裁剪
  void setEnableCrop(bool enable);
  
  // 设置裁剪参数
  void setCropParams(int x, int y, int width, int height);

signals:

  void cameraStarted();

  void playbackStarted();

  void startRecord(const QString &path);

  void stopRecord();

  void saveRecord(const QString &path);

  // 新增max、mid、min三种录制类型的信号
  void startRecordMax(const QString &path);
  
  void startRecordMid(const QString &path);
  
  void startRecordMin(const QString &path);

  void setTH(int v);

  void startButtonClicked();
  
private slots:
  void updateDVSView(QImage img);
  
  // 检测相关槽函数
  void updateDVDetectionView(QImage img);
  void updateDVSDetectionView(QImage img);
  void onDVDetectionResult(QString result);
  void onDVSDetectionResult(QString result);
  void onFinalDecision(QString decision, QDateTime timestamp);
  void onDetectionToggled(bool enabled);
  void onModelLoadResult(bool success, QString message);
  
  // 新增三种类型录制的槽函数
  void handleStartMaxRecord();
  void handleStopMaxRecord();
  
  void handleStartMidRecord();
  void handleStopMidRecord();
  
  void handleStartMinRecord();
  void handleStopMinRecord();
  
  // 播放相关槽（简化版）
  void onPlaybackNewFrame(QImage frame);
  void onPlaybackProgress(double progress);
  void onPlaybackFinished();

  // 批量播放相关槽函数
  void on_selectRootPlaybackDirButton_clicked();
  void onBatchPlaybackStarted(int totalSessions);
  void onBatchPlaybackProgress(int currentSession, int totalSessions, const QString &currentSessionName);
  void onBatchPlaybackFinished();
  void onPlaybackImagePair(QImage dv, QImage dvs);

  // 检测结果图片保存功能
  void saveDetectionResultImage(const QImage &img, const QString &cameraType);

private:
  void closeEvent(QCloseEvent *event) override;
  
  // 初始化检测功能
  void initializeDetection();
  
  // 初始化Modbus连接
  void initializeModbusConnection();
  
  // 加载主配置文件
  void loadMainConfig(const QString &configPath);
  
  // 添加modbusSocket实例
  modbusSocket* socket;
  
  QLabel *videoView;
  QLabel *dvsView;
  int sleepTime = 100;
  QMetaObject::Connection updateDVSViewConnection;
  int showDVSConstruct = 0;
  
  // 配置变量
  bool autoStartCamera = true;
  bool autoStartDetection = true;
  bool autoStartRecording = true;
  bool recordingWithDetection = true;
  
  // 检测相关控件
  QCheckBox *enableDetectionCheckBox;
  QLabel *dvDetectionResultLabel;
  QLabel *dvsDetectionResultLabel;
  QLabel *finalDecisionLabel;
  
  // 检测模块指针
  DetectModule_DV *dvDetector;
  DetectModule_DVS *dvsDetector;
  DualCameraResultManager *resultManager;
  
  // 检测使能标志
  bool detectionEnabled = false;
  
  // 添加三个录制按钮的指针
  QPushButton *maxRecordButton;
  QPushButton *midRecordButton;
  QPushButton *minRecordButton;
  
  // 递归播放相关控件
  QPushButton *selectRootPlaybackDirButton;
  QLabel *rootPlaybackDirLabel;
  QLabel *batchPlaybackStatusLabel;
  
  // 录制状态标记
  bool isMaxRecording = false;
  bool isMidRecording = false;
  bool isMinRecording = false;
  
  // 添加循环旋转按钮指针
  QPushButton *loopRotateButton;
  
  // 添加自动录制按钮指针
  QPushButton *autoRecordingButton;
  
  // 添加新的控件指针
  QPushButton *testDetectFinallyButton;
  // QPushButton *testDetectResButton;
  QLineEdit *xPositionInput;
  QPushButton *xMoveButton;
  QLineEdit *yPositionInput;
  QPushButton *yMoveButton;
  QLineEdit *zPositionInput;  // 添加Z轴输入框指针
  QPushButton *zMoveButton;  // 添加Z轴移动按钮指针
  QPushButton *beginButton;  // 添加begin按钮指针
  // QPushButton *trainingRotateButton;
  
  // 添加8个新按钮：4个带录制功能 + 4个只控制运动
  QPushButton *x1RecordButton;   // X1位置录制按钮
  QPushButton *x2RecordButton;   // X2位置录制按钮
  QPushButton *x3RecordButton;   // X3位置录制按钮
  QPushButton *x4RecordButton;   // X4位置录制按钮
  QPushButton *x1MoveButton;     // X1位置移动按钮
  QPushButton *x2MoveButton;     // X2位置移动按钮
  QPushButton *x3MoveButton;     // X3位置移动按钮
  QPushButton *x4MoveButton;     // X4位置移动按钮
};
#endif // MAINWINDOW_H
