#include "mainwindow.h"
#include "camera/CameraCapture.h"
#include "camera/CameraConfig.h"
#include "camera/CamerasDetector.h"
#include "camera/RecordingConfig.h"
#include "camera/FileSaveManager.h"
#include "camera/DetectionSessionManager.h"
#include "dvs/DVSDataSource.h"
#include "infer/detect/DetectModule_DV.h"
#include "infer/detect/DetectModule_DVS.h"
#include "infer/detect/DualCameraResultManager.h"
#include "unistd.h"
#include "widgets/cameracontrols/CameraControlButtonGroup.h"
#include <QDebug>
#include <QFileDialog>
#include <QImage>
#include <QLine>
#include <QLineEdit>
#include <QPushButton>
#include <QSizePolicy>
#include <QSlider>
#include <QElapsedTimer>
#include <QMetaObject>
#include <QtConcurrent/QtConcurrent>
#include <mcheck.h>
#include <opencv2/opencv.hpp>
#include <thread>
#include <chrono>
#include "socket/modbussocket.h"
#include "PlaybackReader.h"
#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QDateTime>
#include <QProgressBar>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include "camera/ConfigManager.h"

// 注册cv::Mat类型以支持Qt信号槽系统
Q_DECLARE_METATYPE(cv::Mat)

struct Block {
  std::vector<uint8_t> data;
  uint64_t timeStamp;
  uint32_t readIndex = 0;
};
using namespace cv;
//控制是否开启检测选项，在DVSDataSource中引用
bool enableDetect = false;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
#ifdef PROFILE
  ProfilerStart("camera.prof");
#endif

  // 注册cv::Mat类型以支持Qt信号槽系统
  qRegisterMetaType<cv::Mat>("cv::Mat");

  // 启动文件保存管理服务，解决录制时UI卡死问题
  FileSaveManager::getInstance()->startService();

  // 加载录制配置
  loadRecordingConfigFromJson(ConfigManager::getInstance().getConfigPath());
  
  // 加载主配置文件
  loadMainConfig(ConfigManager::getInstance().getConfigPath());
  
  setWindowTitle("演示程序");
  resize(QSize(1280, 720));
  auto layout = new QHBoxLayout();
  auto buttonLayout = new QHBoxLayout();
  auto vboxLayout = new QVBoxLayout();
  auto openCameraButton = new QPushButton(tr("启动相机"));
  autoRecordingButton = new QPushButton(tr("自动录制模式"));
  autoRecordingNoCommButton = new QPushButton(tr("自动录制（无通讯）"));
  // auto q_openButton = new QPushButton(tr("松开"));
  // auto q_closeButton = new QPushButton(tr("抓取"));
  // auto rotateButton = new QPushButton(tr("旋转"));
  // auto reventButton = new QPushButton(tr("x轴旋转"));
  auto x_mobile = new QPushButton(tr("X轴移动"));
  // auto rotate_45 = new QPushButton(tr("旋转45度"));
  auto reset = new QPushButton(tr("复位"));
  auto begin = new QPushButton(tr("开始检测"));
  auto stop_emergency = new QPushButton(tr("即停紧急"));
  // 添加循环旋转按钮
  loopRotateButton = new QPushButton(tr("正反旋转"));
  // 添加选择录制文件按钮
  auto selectRecordButton = new QPushButton(tr("选择录制文件"));
  
  // 添加新的测试按钮
  testDetectFinallyButton = new QPushButton(tr("测试检测完成"));
  // testDetectResButton = new QPushButton(tr("测试检测结果"));
  
  // 添加测试写入0的按钮
  // testDetectFinallyZeroButton = new QPushButton(tr("测试检测完成(0)"));
  // testDetectResZeroButton = new QPushButton(tr("测试检测结果(0)"));
  
  // 添加X轴控制输入框和按钮
  xPositionInput = new QLineEdit();
  xPositionInput->setPlaceholderText("输入X轴位置(13000-33700)");
  xPositionInput->setText("24000"); // 默认中间位置
  xMoveButton = new QPushButton(tr("X轴移动"));
  
  // 添加Y轴控制输入框和按钮
  yPositionInput = new QLineEdit();
  yPositionInput->setPlaceholderText("输入Y轴位置(20000-26000)");
  yPositionInput->setText("26950"); // 默认中间位置
  yMoveButton = new QPushButton(tr("Y轴移动"));
  
  // 添加Z轴控制输入框和按钮
  zPositionInput = new QLineEdit();
  zPositionInput->setPlaceholderText("输入Z轴位置(0-15000)");
  zPositionInput->setText("7500"); // 默认中间位置
  zMoveButton = new QPushButton(tr("Z轴移动"));
  
  // 添加begin按钮
  beginButton = new QPushButton(tr("开始"));
  
  // 将循环旋转改为训练旋转
  // trainingRotateButton = new QPushButton(tr("反复旋转"));
  
  // 添加检测功能控件
  enableDetectionCheckBox = new QCheckBox(tr("启用检测功能"));
  enableDetectionCheckBox->setChecked(false);  // 始终默认为false，不根据配置设置
  connect(enableDetectionCheckBox, &QCheckBox::toggled, this, &MainWindow::onDetectionToggled);
  buttonLayout->addWidget(enableDetectionCheckBox);
  
  // 检测功能默认禁用，不根据配置自动启动
  detectionEnabled = false;
  enableDetect = false;

  buttonLayout->addWidget(openCameraButton);
  buttonLayout->addWidget(autoRecordingButton);
  buttonLayout->addWidget(autoRecordingNoCommButton);
  // buttonLayout->addWidget(q_openButton);
  // buttonLayout->addWidget(q_closeButton);
  // buttonLayout->addWidget(rotateButton);
  // buttonLayout->addWidget(reventButton);
  // buttonLayout->addWidget(connectButton);
  buttonLayout->addWidget(x_mobile);
  // buttonLayout->addWidget(rotate_45);
  buttonLayout->addWidget(reset);
  buttonLayout->addWidget(begin);
  buttonLayout->addWidget(stop_emergency);
  buttonLayout->addWidget(loopRotateButton);
  // 添加选择录制文件按钮到布局
  buttonLayout->addWidget(selectRecordButton);
  // 添加新的控件到布局
  buttonLayout->addWidget(testDetectFinallyButton);
  // buttonLayout->addWidget(testDetectResButton);
  // buttonLayout->addWidget(testDetectFinallyZeroButton);
  // buttonLayout->addWidget(testDetectResZeroButton);
  buttonLayout->addWidget(xPositionInput);
  buttonLayout->addWidget(xMoveButton);
  buttonLayout->addWidget(yPositionInput);
  buttonLayout->addWidget(yMoveButton);
  buttonLayout->addWidget(zPositionInput);  // 添加Z轴输入框到布局
  buttonLayout->addWidget(zMoveButton);  // 添加Z轴移动按钮到布局
  buttonLayout->addWidget(beginButton);  // 添加begin按钮到布局
  // buttonLayout->addWidget(trainingRotateButton);
  // buttonLayout->addWidget(z_mobile);

  // 连接开始检测按钮的点击事件
  connect(begin, &QPushButton::clicked, this, [this]() {
    qDebug() << "开始检测按钮被点击";
    
    // 检查相机是否已启动 - 使用一个更简单的方法
    auto dvsDataSource = DVSDataSource::getInstance();
    
    // 先尝试启动相机（如果尚未启动）
    dvsDataSource->startCamera();
    
    // 给相机一点时间启动
    QTimer::singleShot(500, [this]() {
      // 启用检测功能
      if (!detectionEnabled) {
        enableDetectionCheckBox->setChecked(true);
        qDebug() << "通过开始检测按钮启用了检测功能";
      } else {
        qDebug() << "检测功能已经启用";
      }
    });
  });

  // 创建modbusSocket实例并初始化连接
  initializeModbusConnection();
  
  // 连接各个按钮的槽函数 - 使用多线程避免阻塞UI
  // connect(q_openButton, &QPushButton::clicked, this, [this]() {
  //   qDebug() << "松开按钮被点击，在后台线程执行";
  //   QtConcurrent::run([this]() {
  //     socket->q_open();
  //   });
  // });
  
  // connect(q_closeButton, &QPushButton::clicked, this, [this]() {
  //   qDebug() << "抓取按钮被点击，在后台线程执行";
  //   QtConcurrent::run([this]() {
  //     socket->q_close();
  //   });
  // });

  // connect(rotateButton, &QPushButton::clicked, this, [this]() {
  //   qDebug() << "旋转按钮被点击，在后台线程执行";
  //   QtConcurrent::run([this]() {
  //     socket->rotate();
  //   });
  // });
  
  // 重新连接X轴移动按钮 - 使用多线程执行
  connect(xMoveButton, &QPushButton::clicked, this, [this]() {
    bool ok;
    int xPosition = xPositionInput->text().toInt(&ok);
    if (ok && xPosition >= 13000 && xPosition <= 33700) {
      qDebug() << "X轴移动到位置:" << xPosition << "，在后台线程执行";
      QtConcurrent::run([this, xPosition]() {
      socket->x_basic_move(xPosition);
      });
    } else {
      qDebug() << "X轴位置输入无效，请输入13000-33700之间的数值";
      QMessageBox::warning(this, "输入错误", "请输入13000-33700之间的有效数值");
    }
  });
  
  // 重新连接Y轴移动按钮 - 使用多线程执行
  connect(yMoveButton, &QPushButton::clicked, this, [this]() {
    bool ok;
    int yPosition = yPositionInput->text().toInt(&ok);
    if (ok && yPosition >= 20000 && yPosition <= 29000) {
      qDebug() << "Y轴移动到位置:" << yPosition << "，在后台线程执行";
      QtConcurrent::run([this, yPosition]() {
        socket->y_micore(yPosition);
      });
    } else {
      qDebug() << "Y轴位置输入无效，请输入20000-29000之间的数值";
      QMessageBox::warning(this, "输入错误", "请输入20000-29000之间的有效数值");
    }
  });
  
  // 连接Z轴移动按钮 - 使用多线程执行
  connect(zMoveButton, &QPushButton::clicked, this, [this]() {
    bool ok;
    int zPosition = zPositionInput->text().toInt(&ok);
    if (ok && zPosition >= 0 && zPosition <= 15000) {
      qDebug() << "Z轴移动到位置:" << zPosition << "，在后台线程执行";
      QtConcurrent::run([this, zPosition]() {
        socket->z_mobile(zPosition);
      });
    } else {
      qDebug() << "Z轴位置输入无效，请输入0-15000之间的数值";
      QMessageBox::warning(this, "输入错误", "请输入0-15000之间的有效数值");
    }
  });
  
  // 连接begin按钮 - 使用多线程执行
  connect(beginButton, &QPushButton::clicked, this, [this]() {
    qDebug() << "开始按钮被点击，在后台线程执行";
    QtConcurrent::run([this]() {
      socket->begin();
    });
  });
  
  connect(x_mobile, &QPushButton::clicked, this, [this]() {
    qDebug() << "X轴移动按钮被点击，在后台线程执行";
    QtConcurrent::run([this]() {
      socket->x_mobile();
    });
  });
  
  connect(reset, &QPushButton::clicked, this, [this]() {
    qDebug() << "复位按钮被点击，在后台线程执行";
    QtConcurrent::run([this]() {
      socket->reset();
    });
  });
  
  connect(stop_emergency, &QPushButton::clicked, this, [this]() {
    qDebug() << "紧急停止按钮被点击，在后台线程执行";
    QtConcurrent::run([this]() {
      socket->stop_emergency();
    });
  });

  // 连接正反旋转按钮 - 执行一次性的正反旋转循环，在后台线程执行
  connect(loopRotateButton, &QPushButton::clicked, this, [this]() {
    qDebug() << "正反旋转按钮被点击，在后台线程执行一次性正反旋转";
    QtConcurrent::run([this]() {
      for (int i = 0; i < 3; i++) {
        // 执行一次完整的正反旋转：正向 -> 反向
        qDebug() << "开始执行正反旋转序列";
        socket->rotate();  // 正向旋转
        qDebug() << "正向旋转完成，开始反向旋转";
        socket->revent();  // 反向旋转
        qDebug() << "正反旋转序列完成";
    }
    });
  });

  // 连接新的测试按钮 - 使用多线程执行
  connect(testDetectFinallyButton, &QPushButton::clicked, this, [this]() {
    qDebug() << "测试检测完成按钮被点击，在后台线程执行";
    QtConcurrent::run([this]() {
      socket->testDetectFinally();
    });
  });

  vboxLayout->addLayout(buttonLayout);
  
  // 添加X1-X4位置控制按钮布局
  auto positionButtonLayout = new QHBoxLayout();
  
  // 创建8个按钮：4个录制按钮 + 4个移动按钮
  x1RecordButton = new QPushButton(tr("X1录制"));
  x2RecordButton = new QPushButton(tr("X2录制"));
  x3RecordButton = new QPushButton(tr("X3录制"));
  x4RecordButton = new QPushButton(tr("X4录制"));
  x1MoveButton = new QPushButton(tr("X1移动"));
  x2MoveButton = new QPushButton(tr("X2移动"));
  x3MoveButton = new QPushButton(tr("X3移动"));
  x4MoveButton = new QPushButton(tr("X4移动"));
  
  // 设置按钮样式区分录制和移动功能
  QString recordButtonStyle = "QPushButton { background-color: #4CAF50; color: white; }";
  QString moveButtonStyle = "QPushButton { background-color: #2196F3; color: white; }";
  
  x1RecordButton->setStyleSheet(recordButtonStyle);
  x2RecordButton->setStyleSheet(recordButtonStyle);
  x3RecordButton->setStyleSheet(recordButtonStyle);
  x4RecordButton->setStyleSheet(recordButtonStyle);
  x1MoveButton->setStyleSheet(moveButtonStyle);
  x2MoveButton->setStyleSheet(moveButtonStyle);
  x3MoveButton->setStyleSheet(moveButtonStyle);
  x4MoveButton->setStyleSheet(moveButtonStyle);
  
  // 添加按钮到布局
  positionButtonLayout->addWidget(x1RecordButton);
  positionButtonLayout->addWidget(x1MoveButton);
  positionButtonLayout->addWidget(x2RecordButton);
  positionButtonLayout->addWidget(x2MoveButton);
  positionButtonLayout->addWidget(x3RecordButton);
  positionButtonLayout->addWidget(x3MoveButton);
  positionButtonLayout->addWidget(x4RecordButton);
  positionButtonLayout->addWidget(x4MoveButton);
  
  // 连接按钮信号到槽函数，使用多线程执行
  connect(x1RecordButton, &QPushButton::clicked, this, [this]() {
    qDebug() << "X1录制按钮被点击，在后台线程执行";
    x1RecordButton->setDisabled(true);
    x1RecordButton->setText("X1录制中...");
    QtConcurrent::run([this]() {
      socket->recordAtX1();
      QMetaObject::invokeMethod(this, [this]() {
        x1RecordButton->setDisabled(false);
        x1RecordButton->setText("X1录制");
      }, Qt::QueuedConnection);
    });
  });
  
  connect(x2RecordButton, &QPushButton::clicked, this, [this]() {
    qDebug() << "X2录制按钮被点击，在后台线程执行";
    x2RecordButton->setDisabled(true);
    x2RecordButton->setText("X2录制中...");
    QtConcurrent::run([this]() {
      socket->recordAtX2();
      QMetaObject::invokeMethod(this, [this]() {
        x2RecordButton->setDisabled(false);
        x2RecordButton->setText("X2录制");
      }, Qt::QueuedConnection);
    });
  });
  
  connect(x3RecordButton, &QPushButton::clicked, this, [this]() {
    qDebug() << "X3录制按钮被点击，在后台线程执行";
    x3RecordButton->setDisabled(true);
    x3RecordButton->setText("X3录制中...");
    QtConcurrent::run([this]() {
      socket->recordAtX3();
      QMetaObject::invokeMethod(this, [this]() {
        x3RecordButton->setDisabled(false);
        x3RecordButton->setText("X3录制");
      }, Qt::QueuedConnection);
    });
  });
  
  connect(x4RecordButton, &QPushButton::clicked, this, [this]() {
    qDebug() << "X4录制按钮被点击，在后台线程执行";
    x4RecordButton->setDisabled(true);
    x4RecordButton->setText("X4录制中...");
    QtConcurrent::run([this]() {
      socket->recordAtX4();
      QMetaObject::invokeMethod(this, [this]() {
        x4RecordButton->setDisabled(false);
        x4RecordButton->setText("X4录制");
      }, Qt::QueuedConnection);
    });
  });
  
  connect(x1MoveButton, &QPushButton::clicked, this, [this]() {
    qDebug() << "X1移动按钮被点击，在后台线程执行";
    x1MoveButton->setDisabled(true);
    x1MoveButton->setText("X1移动中...");
    QtConcurrent::run([this]() {
      socket->moveToX1();
      QMetaObject::invokeMethod(this, [this]() {
        x1MoveButton->setDisabled(false);
        x1MoveButton->setText("X1移动");
      }, Qt::QueuedConnection);
    });
  });
  
  connect(x2MoveButton, &QPushButton::clicked, this, [this]() {
    qDebug() << "X2移动按钮被点击，在后台线程执行";
    x2MoveButton->setDisabled(true);
    x2MoveButton->setText("X2移动中...");
    QtConcurrent::run([this]() {
      socket->moveToX2();
      QMetaObject::invokeMethod(this, [this]() {
        x2MoveButton->setDisabled(false);
        x2MoveButton->setText("X2移动");
      }, Qt::QueuedConnection);
    });
  });
  
  connect(x3MoveButton, &QPushButton::clicked, this, [this]() {
    qDebug() << "X3移动按钮被点击，在后台线程执行";
    x3MoveButton->setDisabled(true);
    x3MoveButton->setText("X3移动中...");
    QtConcurrent::run([this]() {
      socket->moveToX3();
      QMetaObject::invokeMethod(this, [this]() {
        x3MoveButton->setDisabled(false);
        x3MoveButton->setText("X3移动");
      }, Qt::QueuedConnection);
    });
  });
  
  connect(x4MoveButton, &QPushButton::clicked, this, [this]() {
    qDebug() << "X4移动按钮被点击，在后台线程执行";
    x4MoveButton->setDisabled(true);
    x4MoveButton->setText("X4移动中...");
    QtConcurrent::run([this]() {
      socket->moveToX4();
      QMetaObject::invokeMethod(this, [this]() {
        x4MoveButton->setDisabled(false);
        x4MoveButton->setText("X4移动");
      }, Qt::QueuedConnection);
    });
  });
  
  vboxLayout->addLayout(positionButtonLayout);
  
  // 添加检测结果显示区域
  auto detectionLayout = new QHBoxLayout();
  dvDetectionResultLabel = new QLabel(tr("DV检测结果: 等待中"));
  dvsDetectionResultLabel = new QLabel(tr("DVS检测结果: 等待中"));
  finalDecisionLabel = new QLabel(tr("最终判决: 等待中"));
  
  detectionLayout->addWidget(dvDetectionResultLabel);
  detectionLayout->addWidget(dvsDetectionResultLabel);
  detectionLayout->addWidget(finalDecisionLabel);
  
  vboxLayout->addLayout(detectionLayout);
  
  QFont font;
  font.setPointSize(15);
  setFont(font);
  
  // 获取DVSDataSource实例（删除DvsCapture的引用）
  auto dvsDataSource = DVSDataSource::getInstance();
  auto dvCapture = CameraCapture::getInstance();
  
  // 初始化检测功能
  initializeDetection();
   
  auto controlButtons = new CameraControlButtonGroup(
      CameraConfig::getInstance().getControlConfig());
  //回放模式选择
  connect(controlButtons, &CameraControlButtonGroup::playbackStarted, this,
          [this, openCameraButton]() { 
            openCameraButton->setDisabled(true);
            
            // 回放开始时，断开原有的DVS数据源更新连接
            if (updateDVSViewConnection) {
              disconnect(updateDVSViewConnection);
              qDebug() << "回放模式：断开原有DVS数据源更新连接";
            }
          });
          
  // 设置DVSDataSource不自动启动
  dvsDataSource->setAutoStart(false);
          
  connect(PlaybackReader::getInstance(), &PlaybackReader::complete, this,
          [this, openCameraButton, dvsDataSource]() { 
            openCameraButton->setDisabled(false);
            
            // 回放结束时，恢复原有的DVS数据源更新连接
            if (!updateDVSViewConnection) {
              updateDVSViewConnection = connect(dvsDataSource, &DVSDataSource::newImage,
                                      this, &MainWindow::updateDVSView);
              qDebug() << "回放结束：恢复原有DVS数据源更新连接";
            }
          });
          
  // 连接回放图像信号，使用统一的显示管理器
  connect(PlaybackReader::getInstance(), &PlaybackReader::nextImagePair, this,
          [this](QImage dvImg, QImage dvsImg) {
            // 回放时临时保存当前显示模式
            DisplayMode originalMode = currentDisplayMode;

            // 回放时强制使用相机视图模式以确保图像显示
            currentDisplayMode = DisplayMode::CAMERA_VIEW;

            // 使用统一的显示管理器处理DV图像
            if (!dvImg.isNull()) {
              QMetaObject::invokeMethod(this, "updateCameraDisplay",
                                       Qt::QueuedConnection,
                                       Q_ARG(QImage, dvImg),
                                       Q_ARG(QString, "DV"));
            }

            // 使用统一的显示管理器处理DVS图像
            if (!dvsImg.isNull()) {
              QMetaObject::invokeMethod(this, "updateCameraDisplay",
                                       Qt::QueuedConnection,
                                       Q_ARG(QImage, dvsImg),
                                       Q_ARG(QString, "DVS"));
            }

            // 恢复原始显示模式
            currentDisplayMode = originalMode;
          });

  // 连接选择录制文件按钮
  connect(selectRecordButton, &QPushButton::clicked, this, [this, dvsDataSource]() {
    // 首先停止相机，避免同时运行
    dvsDataSource->stopCamera();
    
    // 打开文件选择对话框
    QString recordFile = QFileDialog::getOpenFileName(this, tr("选择录制文件"), 
                                                    "", tr("录制文件 (*.raw *.bin)"));
    if (recordFile.isEmpty()) {
      return;
    }
    
    qDebug() << "选择了录制文件:" << recordFile;
    
    // 设置回放参数
    PlaybackReader::PlaybackParams params;
    params.visualizeOnly = true;  // 仅可视化，不保存图像
    
    // 只支持RAW文件
    if (recordFile.endsWith(".raw", Qt::CaseInsensitive)) {
      params.rawFilePath = recordFile;
      qDebug() << "检测到RAW文件，使用RAW回放方式";
    } else {
      qDebug() << "不支持的文件类型，只支持RAW文件:" << recordFile;
      return;
    }
    
    // 断开原有的DVS数据源更新连接
    if (updateDVSViewConnection) {
      disconnect(updateDVSViewConnection);
    }
    
    // 启动可视化回放
    if (!params.rawFilePath.isEmpty()) {
      PlaybackReader::getInstance()->visualizeRawFile(params);
    }
  });

  // 修改相机启动相关的连接，使用DVSDataSource
  // 启动相机时开始读取dvs数据
  connect(openCameraButton, &QPushButton::clicked, dvsDataSource,
          &DVSDataSource::startCamera);
  
  // 连接自动录制模式按钮
  connect(autoRecordingButton, &QPushButton::clicked, this, [this]() {
    qDebug() << "自动录制模式按钮被点击";

    // 询问用户确认
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "自动录制确认",
        "即将开始自动录制25张盘片的流程，每张盘片将在3个位置(min/mid/max)进行录制。\n"
        "此过程将耗时较长，确认开始吗？",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
      qDebug() << "用户确认开始自动录制，在后台线程执行";
      autoRecordingButton->setDisabled(true);
      autoRecordingButton->setText("自动录制中...");

      QtConcurrent::run([this]() {
        socket->performAutoRecording();
      });
      } else {
      qDebug() << "用户取消自动录制";
    }
  });

  // 连接无通讯自动录制模式按钮
  connect(autoRecordingNoCommButton, &QPushButton::clicked, this, [this]() {
    qDebug() << "无通讯自动录制模式按钮被点击";

    // 询问用户确认
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "无通讯自动录制确认",
        "即将开始无通讯模式的自动录制测试，将模拟25张盘片的录制流程。\n"
        "此模式不会进行实际的硬件操作，仅测试软件逻辑和UI响应性。\n"
        "确认开始测试吗？",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
      qDebug() << "用户确认开始无通讯自动录制测试，在后台线程执行";
      autoRecordingNoCommButton->setDisabled(true);
      autoRecordingNoCommButton->setText("无通讯录制中...");

      QtConcurrent::run([this]() {
        socket->performAutoRecordingNoComm();
      });
    } else {
      qDebug() << "用户取消无通讯自动录制测试";
    }
  });
  
  // 启动相机时，开始发出cameraStarted信号，让上部相机控制栏作出响应
  connect(dvsDataSource, &DVSDataSource::cameraStarted, controlButtons,
          &CameraControlButtonGroup::camerasStarted);
  
  // 启动相机后，启动相机按钮失效
  connect(dvsDataSource, &DVSDataSource::cameraStarted, this, [openCameraButton]() {
    openCameraButton->setDisabled(true);
  });

  vboxLayout->addWidget(controlButtons);
  
  // 调试信息：检查DV摄像头启动条件
  bool dvEnabled = CameraConfig::getInstance().getControlConfig().dv;
  bool camerasDetected = !CamerasDetector::getInstance()->getCamerasIndex().empty();
  auto cameraIndexes = CamerasDetector::getInstance()->getCamerasIndex();
  qDebug() << "DV启动条件检查:";
  qDebug() << "  - DV配置启用:" << dvEnabled;
  qDebug() << "  - 检测到摄像头:" << camerasDetected;
  qDebug() << "  - 摄像头索引列表:" << cameraIndexes.size() << "个";
  for (int i = 0; i < cameraIndexes.size(); i++) {
    qDebug() << "    摄像头索引[" << i << "]:" << cameraIndexes[i];
  }
  
  if (dvEnabled && camerasDetected) {
    qDebug() << "dv";
    videoView = new QLabel();
    layout->addWidget(videoView);
    
    // 修改初始化完成的连接，使用DVSDataSource
    connect(dvsDataSource, &DVSDataSource::cameraStarted, this, [dvCapture, cameraIndexes]() {
      qDebug() << "dv start ";
      // 使用检测到的第一个摄像头索引
      int cameraIndex = cameraIndexes.empty() ? 0 : cameraIndexes[0];
      qDebug() << "使用摄像头索引:" << cameraIndex;
      dvCapture->startCapture(cameraIndex);
    });
    
    connect(dvCapture, &CameraCapture::captureImage, this,
            [this](cv::Mat *img) {
              qDebug() << "DV图像接收 - 线程ID:" << QThread::currentThreadId();
              qDebug() << "DV图像接收 - 指针地址:" << (void*)img;

              if (!img) {
                qDebug() << "DV检测: 图像指针为空";
                return;
              }

              // 立即进行深拷贝以避免多线程问题
              cv::Mat safeCopy;
              try {
                img->copyTo(safeCopy);
                qDebug() << "DV图像深拷贝成功 - 尺寸:" << safeCopy.rows << "x" << safeCopy.cols
                         << "类型:" << safeCopy.type() << "通道:" << safeCopy.channels();
              } catch (const cv::Exception& e) {
                qDebug() << "DV图像深拷贝失败:" << e.what();
                delete img;  // 清理原始指针
                return;
              }

              // 清理原始指针
              delete img;

              // 转换为QImage用于显示
              QImage qimg(safeCopy.data, safeCopy.cols, safeCopy.rows, safeCopy.step, QImage::Format_RGB888);

              // 使用统一的显示管理器更新相机画面（线程安全）
              QMetaObject::invokeMethod(this, "updateCameraDisplay",
                                       Qt::QueuedConnection,
                                       Q_ARG(QImage, qimg),
                                       Q_ARG(QString, "DV"));

              // 为实时相机生成文件名
              uint64_t timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                  std::chrono::system_clock::now().time_since_epoch()).count();
              currentDVFilename = QString("%1_dv.png").arg(timestamp);

              // 如果检测功能启用，将图像传递给独立的检测处理方法
              if (detectionEnabled) {
                // 使用Qt::QueuedConnection确保线程安全，统一架构
                QMetaObject::invokeMethod(this, "processDVImageForDetection",
                                         Qt::QueuedConnection, Q_ARG(cv::Mat, safeCopy));
              }
            }, Qt::QueuedConnection);
    
    connect(dvCapture, &CameraCapture::initFinished, this,
            &MainWindow::cameraStarted);
  } else {
    // 连接DVSDataSource的相机启动信号
    connect(dvsDataSource, &DVSDataSource::cameraStarted, this,
            &MainWindow::cameraStarted);
  }
  
  dvsView = new QLabel();
  layout->addWidget(dvsView);
  updateDVSViewConnection = connect(dvsDataSource, &DVSDataSource::newImage,
                                    this, &MainWindow::updateDVSView);

  vboxLayout->addLayout(layout, 1);

  vboxLayout->setAlignment(layout, Qt::AlignVCenter);
  vboxLayout->addWidget(openCameraButton, 0,
                        Qt::AlignHCenter | Qt::AlignBottom);

  // todo 使用多个按钮控制显示状态
  // todo 不同的状态运行不同的模型，以提升性能
  auto switchDVSView = new QPushButton(tr("DVS重构/原图显示切换"));
  if (CameraConfig::getInstance().getControlConfig().construct)
    vboxLayout->addWidget(switchDVSView, 0, Qt::AlignHCenter | Qt::AlignBottom);
    
  // 添加DV显示模式切换按钮
  auto switchDVDisplayMode = new QPushButton(tr("原图/裁剪图切换"));
  vboxLayout->addWidget(switchDVDisplayMode, 0, Qt::AlignHCenter | Qt::AlignBottom);
  connect(switchDVDisplayMode, &QPushButton::clicked, this, []() {
    auto dvConfig = DVDisplayConfig::getInstance();
    // 切换显示模式
    if (dvConfig->getDisplayMode() == "original") {
      dvConfig->setDisplayMode("cropped");
    } else {
      dvConfig->setDisplayMode("original");
    }
  });
  
  connect(switchDVSView, &QPushButton::clicked, this, [this]() {
    disconnect(updateDVSViewConnection);
    this->showDVSConstruct = (this->showDVSConstruct + 1) % 2;
    switch (this->showDVSConstruct) {
    case 0:
      updateDVSViewConnection = connect(DVSDataSource::getInstance(), &DVSDataSource::newImage,
                                        this, &MainWindow::updateDVSView);
      break;
    case 1:
      // 显示空白图像或不显示
      this->dvsView->clear();
      break;
    }
  });

  connect(this, &MainWindow::playbackStarted, this,
          [openCameraButton]() { openCameraButton->setEnabled(false); });

  auto recordLayout = new QHBoxLayout;
  auto startRecordButton = new QPushButton(tr("开始录制"));
  auto saveRecordingButton = new QPushButton(tr("保存"));
  startRecordButton->setDisabled(true);
  connect(this, &MainWindow::cameraStarted, this,
          [startRecordButton]() { startRecordButton->setEnabled(true); });
  saveRecordingButton->setDisabled(true);
  
  connect(startRecordButton, &QPushButton::clicked, this, [this, startRecordButton, saveRecordingButton]() {
    startRecordButton->setDisabled(true);
    startRecordButton->setText(tr("录制中"));
    saveRecordingButton->setEnabled(true);
    
    // 禁用三个新录制按钮
    maxRecordButton->setDisabled(true);
    midRecordButton->setDisabled(true);
    minRecordButton->setDisabled(true);
    
    auto path = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH_mm_ss");
    qDebug() << "Starting record with path:" << path;
    emit this->startRecord(path);
  });

  connect(saveRecordingButton, &QPushButton::clicked, this, [this, startRecordButton, saveRecordingButton]() {
    qDebug() << "Stopping record";
    emit this->stopRecord();
    emit this->saveRecord("");
    saveRecordingButton->setDisabled(true);
    startRecordButton->setEnabled(true);
    startRecordButton->setText(tr("开始录制"));
    
    // 重新启用三个新录制按钮
    maxRecordButton->setEnabled(true);
    midRecordButton->setEnabled(true);
    minRecordButton->setEnabled(true);
  });

  recordLayout->addWidget(startRecordButton);
  recordLayout->addWidget(saveRecordingButton);
  vboxLayout->addLayout(recordLayout);
  
  // 添加三个新的录制按钮
  auto specialRecordLayout = new QHBoxLayout();
  
  // Max录制按钮
  maxRecordButton = new QPushButton(tr("录制MAX"));
  maxRecordButton->setDisabled(true);
  connect(this, &MainWindow::cameraStarted, this,
          [this]() { this->maxRecordButton->setEnabled(true); });
  connect(maxRecordButton, &QPushButton::clicked, this, &MainWindow::handleStartMaxRecord);
  specialRecordLayout->addWidget(maxRecordButton);
  
  // Mid录制按钮
  midRecordButton = new QPushButton(tr("录制MID"));
  midRecordButton->setDisabled(true);
  connect(this, &MainWindow::cameraStarted, this,
          [this]() { this->midRecordButton->setEnabled(true); });
  connect(midRecordButton, &QPushButton::clicked, this, &MainWindow::handleStartMidRecord);
  specialRecordLayout->addWidget(midRecordButton);
  
  // Min录制按钮
  minRecordButton = new QPushButton(tr("录制MIN"));
  minRecordButton->setDisabled(true);
  connect(this, &MainWindow::cameraStarted, this,
          [this]() { this->minRecordButton->setEnabled(true); });
  connect(minRecordButton, &QPushButton::clicked, this, &MainWindow::handleStartMinRecord);
  specialRecordLayout->addWidget(minRecordButton);
  
  vboxLayout->addLayout(specialRecordLayout);

  // 创建批量播放区域（简化版，只保留递归播放功能）
  auto batchPlaybackLayout = new QHBoxLayout();
  selectRootPlaybackDirButton = new QPushButton(tr("选择主目录递归播放"));
  rootPlaybackDirLabel = new QLabel(tr("未选择主目录"));
  rootPlaybackDirLabel->setMinimumWidth(400);
  batchPlaybackStatusLabel = new QLabel(tr("播放状态: 未开始"));
  batchPlaybackStatusLabel->setMinimumWidth(300);

  batchPlaybackLayout->addWidget(new QLabel(tr("递归播放:")));
  batchPlaybackLayout->addWidget(rootPlaybackDirLabel);
  batchPlaybackLayout->addWidget(selectRootPlaybackDirButton);
  batchPlaybackLayout->addWidget(batchPlaybackStatusLabel);

  // 将播放控件添加到主布局
  vboxLayout->addLayout(batchPlaybackLayout);

  // 连接信号槽（只保留递归播放相关）
  connect(selectRootPlaybackDirButton, &QPushButton::clicked, this, &MainWindow::on_selectRootPlaybackDirButton_clicked);

  // 连接PlaybackReader的信号（简化版，主要用于递归播放）
  connect(PlaybackReader::getInstance(), &PlaybackReader::newFrame, this, &MainWindow::onPlaybackNewFrame);
  connect(PlaybackReader::getInstance(), &PlaybackReader::playbackFinished, this, &MainWindow::onPlaybackFinished);
  connect(PlaybackReader::getInstance(), &PlaybackReader::playbackProgress, this, &MainWindow::onPlaybackProgress);

  // 连接递归播放信号
  connect(PlaybackReader::getInstance(), &PlaybackReader::batchPlaybackStarted, this, &MainWindow::onBatchPlaybackStarted);
  connect(PlaybackReader::getInstance(), &PlaybackReader::batchPlaybackProgress, this, &MainWindow::onBatchPlaybackProgress);
  connect(PlaybackReader::getInstance(), &PlaybackReader::batchPlaybackFinished, this, &MainWindow::onBatchPlaybackFinished);

  // 连接PlaybackReader的nextImagePair信号到检测模块
  connect(PlaybackReader::getInstance(), &PlaybackReader::nextImagePair, this, &MainWindow::onPlaybackImagePair);

  // 连接带文件名的图像对信号以更新当前文件名
  connect(PlaybackReader::getInstance(), &PlaybackReader::nextImagePairWithFilenames, this,
          [this](QImage dv, QImage dvs, QString dvFilename, QString dvsFilename) {
            // 更新当前处理的文件名
            currentDVFilename = dvFilename;
            currentDVSFilename = dvsFilename;
          });

  auto *widget = new QWidget();
  widget->setLayout(vboxLayout);
  setCentralWidget(widget);

  auto sourceInstance = DVSDataSource::getInstance();
  connect(this, &MainWindow::startRecord, sourceInstance,
          &DVSDataSource::startRecord);
  connect(this, &MainWindow::stopRecord, sourceInstance,
          &DVSDataSource::stopRecord);
  connect(this, &MainWindow::startRecord, CameraCapture::getInstance(),
          &CameraCapture::startRecord);
  connect(this, &MainWindow::stopRecord, CameraCapture::getInstance(),
          &CameraCapture::stopRecord);

  // 连接录制信号到检测会话管理 - 优化为仅在必要时重置
  connect(this, &MainWindow::startRecord, this, [this](const QString& path) {
    // 录制开始时，只有在检测会话配置为创建独立文件夹时才重置
    if (detectionEnabled) {
      QtConcurrent::run([this, path]() {
        auto sessionManager = DetectionSessionManager::getInstance();
        const auto& config = sessionManager->getConfig();

        // 只有在配置为不创建会话文件夹时才重置（使用录制会话）
        if (!config.createSessionFolders) {
          sessionManager->resetDetectionSession();
          qDebug() << "录制开始，检测会话已重置以使用录制会话:" << path;
        } else {
          qDebug() << "录制开始，检测会话保持独立，不重置:" << path;
        }
      });
    }
  });

  connect(this, &MainWindow::stopRecord, this, [this]() {
    // 录制停止时，只有在使用录制会话的情况下才重置为独立会话
    if (detectionEnabled) {
      QtConcurrent::run([this]() {
        auto sessionManager = DetectionSessionManager::getInstance();
        const auto& config = sessionManager->getConfig();

        // 只有在配置为不创建会话文件夹时才重置（恢复独立会话）
        if (!config.createSessionFolders) {
          sessionManager->resetDetectionSession();
          qDebug() << "录制停止，检测会话已重置为独立会话";
        } else {
          qDebug() << "录制停止，检测会话保持独立，不重置";
        }
      });
    }
  });
}

MainWindow::~MainWindow() {
  // 停止文件保存管理服务
  FileSaveManager::getInstance()->stopService();

  // 清理socket实例
  if (socket) {
    delete socket;
    socket = nullptr;
  }
  
  // 清理检测模块
  if (dvDetector) {
    // 断开信号连接
    disconnect(dvDetector, nullptr, this, nullptr);
    dvDetector = nullptr;
  }
  
  if (dvsDetector) {
    // 断开信号连接
    disconnect(dvsDetector, nullptr, this, nullptr);
    dvsDetector = nullptr;
  }
  
  if (resultManager) {
    // 断开信号连接
    disconnect(resultManager, nullptr, this, nullptr);
    resultManager = nullptr;
  }
  
  qDebug() << "MainWindow析构函数完成";
}

void MainWindow::closeEvent(QCloseEvent *event) {
#ifdef PROFILE
  ProfilerStop();
  exit(0);
#else
  _exit(0);
#endif
}
void MainWindow::updateDVSView(QImage img) {
  // 使用统一的显示管理器更新相机画面
  updateCameraDisplay(img, "DVS");

  // 为实时DVS相机生成文件名
  uint64_t timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  currentDVSFilename = QString("%1_dvs.png").arg(timestamp);

  // 如果检测功能启用，将图像传递给独立的检测处理方法
  if (detectionEnabled) {
    // 使用Qt::QueuedConnection确保线程安全
    QMetaObject::invokeMethod(this, "processDVSImageForDetection",
                             Qt::QueuedConnection, Q_ARG(QImage, img));
  }
}

// 统一的图像格式转换工具方法
std::shared_ptr<cv::Mat> MainWindow::convertQImageToBGRMat(const QImage& img) {
  if (img.isNull() || img.width() <= 0 || img.height() <= 0) {
    return nullptr;
  }

  try {
    // 确保图像格式为RGB888
    QImage rgbImg = img.convertToFormat(QImage::Format_RGB888);

    // 检查转换是否成功
    if (rgbImg.isNull() || rgbImg.width() <= 0 || rgbImg.height() <= 0) {
      return nullptr;
    }

    // 创建cv::Mat的安全方式
    cv::Mat detectImg(rgbImg.height(), rgbImg.width(), CV_8UC3);

    // 检查cv::Mat是否创建成功
    if (detectImg.empty() || !detectImg.data) {
      return nullptr;
    }

    // 手动复制数据，避免内存管理问题
    bool copySuccess = true;
    try {
      for (int y = 0; y < rgbImg.height(); ++y) {
        const uchar* srcLine = rgbImg.constScanLine(y);
        uchar* dstLine = detectImg.ptr<uchar>(y);
        if (!srcLine || !dstLine) {
          copySuccess = false;
          break;
        }
        for (int x = 0; x < rgbImg.width(); ++x) {
          // RGB -> BGR
          dstLine[x * 3 + 0] = srcLine[x * 3 + 2]; // B
          dstLine[x * 3 + 1] = srcLine[x * 3 + 1]; // G
          dstLine[x * 3 + 2] = srcLine[x * 3 + 0]; // R
        }
      }
    } catch (...) {
      copySuccess = false;
    }

    if (!copySuccess) {
      return nullptr;
    }

    // 使用shared_ptr管理内存，确保线程安全
    return std::make_shared<cv::Mat>(detectImg.clone());

  } catch (const cv::Exception& e) {
    qDebug() << "图像格式转换OpenCV异常:" << e.what();
    return nullptr;
  } catch (const std::exception& e) {
    qDebug() << "图像格式转换异常:" << e.what();
    return nullptr;
  } catch (...) {
    qDebug() << "图像格式转换未知异常";
    return nullptr;
  }
}

// ==================== 显示状态管理器实现 ====================

// 检查是否应该更新显示（帧率限制）
bool MainWindow::shouldUpdateDisplay(const QString& cameraType) {
  QElapsedTimer* timer = nullptr;
  bool* timerInitialized = nullptr;

  if (cameraType == "DV") {
    timer = &dvDisplayTimer;
    timerInitialized = &dvDisplayTimerInitialized;
  } else if (cameraType == "DVS") {
    timer = &dvsDisplayTimer;
    timerInitialized = &dvsDisplayTimerInitialized;
  } else {
    return false;
  }

  // 初始化计时器
  if (!*timerInitialized) {
    timer->start();
    *timerInitialized = true;
    return false; // 第一次调用不更新，只初始化计时器
  }

  // 帧率限制：每15ms最多更新一次
  if (timer->elapsed() < DISPLAY_UPDATE_INTERVAL_MS) {
    return false;
  }
  timer->restart();
  return true;
}

// 统一的图像处理方法（镜像翻转、缩放）
QImage MainWindow::applyImageProcessing(const QImage& img, const QString& cameraType) {
  if (img.isNull()) {
    return QImage();
  }

  auto configInstance = CameraConfig::getInstance();
  QImage processedImg;

  if (cameraType == "DV") {
    processedImg = img.mirrored(
        configInstance.getDVViewHorizontalFlip(),
        configInstance.getDVViewVerticalFlip());
  } else if (cameraType == "DVS") {
    processedImg = img.mirrored(
        configInstance.getDVSViewHorizontalFlip(),
        configInstance.getDVSViewVerticalFlip());
  } else {
    processedImg = img;
  }

  return processedImg;
}

// 统一的相机画面显示更新方法
void MainWindow::updateCameraDisplay(const QImage& img, const QString& cameraType) {
  // 检查帧率限制
  if (!shouldUpdateDisplay(cameraType)) {
    return;
  }

  // 在检测模式下，只有检测结果才显示，相机画面不显示
  // 但是如果当前显示模式是相机视图，则始终显示（用于回放等场景）
  if (detectionEnabled && currentDisplayMode == DisplayMode::DETECTION_VIEW) {
    return;
  }

  // 应用图像处理
  QImage processedImg = applyImageProcessing(img, cameraType);
  if (processedImg.isNull()) {
    return;
  }

  // 根据相机类型更新对应的显示控件
  QLabel* targetView = nullptr;
  if (cameraType == "DV" && videoView) {
    targetView = videoView;
  } else if (cameraType == "DVS" && dvsView) {
    targetView = dvsView;
  }

  if (targetView) {
    targetView->setPixmap(QPixmap::fromImage(processedImg.scaledToWidth(targetView->width())));
  }
}

// 统一的检测结果显示更新方法
void MainWindow::updateDetectionDisplay(const QImage& img, const QString& cameraType) {
  // 只有在检测模式下才显示检测结果
  if (!detectionEnabled) {
    return;
  }

  // 应用图像处理
  QImage processedImg = applyImageProcessing(img, cameraType);
  if (processedImg.isNull()) {
    return;
  }

  // 根据相机类型更新对应的显示控件
  QLabel* targetView = nullptr;
  if (cameraType == "DV" && videoView) {
    targetView = videoView;
  } else if (cameraType == "DVS" && dvsView) {
    targetView = dvsView;
  }

  if (targetView) {
    targetView->setPixmap(QPixmap::fromImage(processedImg.scaledToWidth(targetView->width())));

    // 保存检测结果图片
    saveDetectionResultImage(img, cameraType);
  }
}

// 独立的DV图像检测处理方法
void MainWindow::processDVImageForDetection(cv::Mat img) {
  if (!detectionEnabled || !dvDetector) {
    return;
  }

  // 检查图像是否有效
  if (img.empty() || !img.data || img.rows <= 0 || img.cols <= 0) {
    return;
  }

  try {
    // 检查图像是否为有效的8位图像
    if (img.depth() != CV_8U) {
      return;
    }

    if (img.channels() != 3) {
      return;
    }

    // 创建要检测的图像副本
    cv::Mat detectImg;
    img.copyTo(detectImg);

    // 检查转换后的图像
    if (detectImg.empty() || !detectImg.data) {
      return;
    }

    // 使用shared_ptr管理内存，确保线程安全
    std::shared_ptr<cv::Mat> imgPtr = std::make_shared<cv::Mat>(detectImg);
    dvDetector->enqueue(imgPtr);

  } catch (const cv::Exception& e) {
    qDebug() << "DV检测OpenCV异常:" << e.what();
  } catch (const std::exception& e) {
    qDebug() << "DV检测图像处理异常:" << e.what();
  } catch (...) {
    qDebug() << "DV检测图像处理未知异常";
  }
}

// 独立的DVS图像检测处理方法
void MainWindow::processDVSImageForDetection(QImage img) {
  if (!detectionEnabled || !dvsDetector) {
    return;
  }

  // 使用统一的图像格式转换方法
  std::shared_ptr<cv::Mat> imgPtr = convertQImageToBGRMat(img);

  if (imgPtr && !imgPtr->empty()) {
    dvsDetector->enqueue(imgPtr);
  }
}

void MainWindow::setEnableCrop(bool enable) {
  CameraCapture::getInstance()->setEnableCrop(enable);
}

void MainWindow::setCropParams(int x, int y, int width, int height) {
  CameraCapture::getInstance()->setCropParams(x, y, width, height);
}

void MainWindow::handleStartMaxRecord() {
  if (!isMaxRecording) {
    // 开始录制
    isMaxRecording = true;
    maxRecordButton->setText(tr("停止MAX录制"));
    
    // 禁用其他录制按钮
    midRecordButton->setDisabled(true);
    minRecordButton->setDisabled(true);
    
    auto path = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH_mm_ss") + "_max";
    qDebug() << "Starting MAX record with path:" << path;
    emit this->startRecord(path);
  } else {
    // 停止录制
    handleStopMaxRecord();
  }
}

void MainWindow::handleStopMaxRecord() {
  if (isMaxRecording) {
    qDebug() << "Stopping MAX record";
    emit this->stopRecord();
    emit this->saveRecord("");
    isMaxRecording = false;
    maxRecordButton->setText(tr("录制MAX"));
    
    // 重新启用其他录制按钮
    midRecordButton->setEnabled(true);
    minRecordButton->setEnabled(true);
  }
}

void MainWindow::handleStartMidRecord() {
  if (!isMidRecording) {
    // 开始录制
    isMidRecording = true;
    midRecordButton->setText(tr("停止MID录制"));
    
    // 禁用其他录制按钮
    maxRecordButton->setDisabled(true);
    minRecordButton->setDisabled(true);
    
    auto path = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH_mm_ss") + "_mid";
    qDebug() << "Starting MID record with path:" << path;
    emit this->startRecord(path);
  } else {
    // 停止录制
    handleStopMidRecord();
  }
}

void MainWindow::handleStopMidRecord() {
  if (isMidRecording) {
    qDebug() << "Stopping MID record";
    emit this->stopRecord();
    emit this->saveRecord("");
    isMidRecording = false;
    midRecordButton->setText(tr("录制MID"));
    
    // 重新启用其他录制按钮
    maxRecordButton->setEnabled(true);
    minRecordButton->setEnabled(true);
  }
}

void MainWindow::handleStartMinRecord() {
  if (!isMinRecording) {
    // 开始录制
    isMinRecording = true;
    minRecordButton->setText(tr("停止MIN录制"));
    
    // 禁用其他录制按钮
    maxRecordButton->setDisabled(true);
    midRecordButton->setDisabled(true);
    
    auto path = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH_mm_ss") + "_min";
    qDebug() << "Starting MIN record with path:" << path;
    emit this->startRecord(path);
  } else {
    // 停止录制
    handleStopMinRecord();
  }
}

void MainWindow::handleStopMinRecord() {
  if (isMinRecording) {
    qDebug() << "Stopping MIN record";
    emit this->stopRecord();
    emit this->saveRecord("");
    isMinRecording = false;
    minRecordButton->setText(tr("录制MIN"));
    
    // 重新启用其他录制按钮
    maxRecordButton->setEnabled(true);
    midRecordButton->setEnabled(true);
  }
}

// 播放回调方法（使用统一的显示管理器）
void MainWindow::onPlaybackNewFrame(QImage frame) {
  // 使用统一的显示管理器处理单帧回放（主要用于DVS单独回放）
  if (!frame.isNull()) {
    // 回放时临时保存当前显示模式并强制使用相机视图
    DisplayMode originalMode = currentDisplayMode;
    currentDisplayMode = DisplayMode::CAMERA_VIEW;

    QMetaObject::invokeMethod(this, "updateCameraDisplay",
                             Qt::QueuedConnection,
                             Q_ARG(QImage, frame),
                             Q_ARG(QString, "DVS"));

    // 恢复原始显示模式
    currentDisplayMode = originalMode;
  }
}

void MainWindow::onPlaybackFinished() {
  // 兼容性方法，主要用于单文件回放完成
}

void MainWindow::onPlaybackProgress(double progress) {
  // 兼容性方法，用于单文件回放进度
  (void)progress; // 避免未使用参数的警告
}

void MainWindow::on_selectRootPlaybackDirButton_clicked() {
  QString rootDir = QFileDialog::getExistingDirectory(this, tr("选择递归播放主目录"),
                                                   QDir::currentPath());
  if (!rootDir.isEmpty()) {
    qDebug() << "选择的递归播放主目录:" << rootDir;
    rootPlaybackDirLabel->setText(rootDir);

    // 禁用播放按钮，防止重复点击
    selectRootPlaybackDirButton->setEnabled(false);

    // 开始批量播放
    PlaybackReader::getInstance()->startBatchPlayback(rootDir);
  }
}

void MainWindow::onBatchPlaybackStarted(int totalSessions) {
  batchPlaybackStatusLabel->setText(tr("播放状态: 开始 (共%1个会话)").arg(totalSessions));
}

void MainWindow::onBatchPlaybackProgress(int currentSession, int totalSessions, const QString &currentSessionName) {
  batchPlaybackStatusLabel->setText(tr("播放状态: %1/%2 - %3").arg(currentSession).arg(totalSessions).arg(currentSessionName));
}

void MainWindow::onBatchPlaybackFinished() {
  batchPlaybackStatusLabel->setText(tr("播放状态: 已完成"));

  // 恢复按钮状态
  selectRootPlaybackDirButton->setEnabled(true);
}

void MainWindow::onPlaybackImagePair(QImage dv, QImage dvs) {
  // 回放时临时保存当前显示模式并强制使用相机视图
  DisplayMode originalMode = currentDisplayMode;
  currentDisplayMode = DisplayMode::CAMERA_VIEW;

  // 处理DV图像 - 完全模仿CameraCapture的处理流程
  if (!dv.isNull()) {
    // 模仿CameraCapture::captureImage信号的处理流程
    try {
      // 将QImage转换为cv::Mat，模仿海康相机的输出格式
      cv::Mat dvMat;
      if (dv.format() != QImage::Format_RGB888) {
        dv = dv.convertToFormat(QImage::Format_RGB888);
      }

      // 创建cv::Mat，注意OpenCV使用BGR格式
      dvMat = cv::Mat(dv.height(), dv.width(), CV_8UC3);

      // 转换QImage(RGB)到cv::Mat(BGR) - 模仿海康相机的颜色格式
      for (int y = 0; y < dv.height(); ++y) {
        const uchar* srcLine = dv.constScanLine(y);
        uchar* dstLine = dvMat.ptr<uchar>(y);
        for (int x = 0; x < dv.width(); ++x) {
          dstLine[x * 3 + 0] = srcLine[x * 3 + 2]; // B
          dstLine[x * 3 + 1] = srcLine[x * 3 + 1]; // G
          dstLine[x * 3 + 2] = srcLine[x * 3 + 0]; // R
        }
      }

      // 应用与真实相机相同的图像处理流程
      auto configInstance = CameraConfig::getInstance();

      // 转换为RGB用于显示（模仿CameraCapture的处理）
      cv::Mat displayMat;
      cv::cvtColor(dvMat, displayMat, cv::COLOR_BGR2RGB);

      // 创建QImage用于显示
      QImage displayImg(displayMat.data, displayMat.cols, displayMat.rows,
                       displayMat.step, QImage::Format_RGB888);

      // 使用统一的显示管理器更新相机画面（线程安全）
      QMetaObject::invokeMethod(this, "updateCameraDisplay",
                               Qt::QueuedConnection,
                               Q_ARG(QImage, displayImg),
                               Q_ARG(QString, "DV"));

      // 如果检测功能启用，将图像传递给统一的检测处理方法
      if (detectionEnabled) {
        // 转换为RGB格式给检测模块（模仿CameraCapture的captureImage信号）
        cv::Mat detectMat;
        cv::cvtColor(dvMat, detectMat, cv::COLOR_BGR2RGB);

        // 使用统一的检测处理架构
        QMetaObject::invokeMethod(this, "processDVImageForDetection",
                                 Qt::QueuedConnection, Q_ARG(cv::Mat, detectMat));
      }

    } catch (const std::exception& e) {
      qDebug() << "DV回放图像处理异常:" << e.what();
    }
  }

  // 处理DVS图像 - 使用统一的显示管理器
  if (!dvs.isNull()) {
    try {
      // 使用统一的显示管理器更新相机画面（线程安全）
      QMetaObject::invokeMethod(this, "updateCameraDisplay",
                               Qt::QueuedConnection,
                               Q_ARG(QImage, dvs),
                               Q_ARG(QString, "DVS"));

      // 如果检测功能启用，将DVS图像传递给统一的检测处理方法
      if (detectionEnabled) {
        // 使用统一的检测处理架构
        QMetaObject::invokeMethod(this, "processDVSImageForDetection",
                                 Qt::QueuedConnection, Q_ARG(QImage, dvs));
      }
    } catch (const std::exception& e) {
      qDebug() << "DVS回放图像处理异常:" << e.what();
    }
  }

  // 恢复原始显示模式
  currentDisplayMode = originalMode;
}

void MainWindow::initializeDetection() {
  // 首先检查配置文件中是否启用了检测功能
  QString configPath = ConfigManager::getInstance().getConfigPath();
  QFile file(configPath);
  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() << "无法打开配置文件，使用默认检测设置:" << configPath;
  } else {
    QByteArray jsonData = file.readAll();
    QJsonDocument document = QJsonDocument::fromJson(jsonData);
    
    if (!document.isNull() && document.isObject()) {
      QJsonObject rootObj = document.object();
      if (rootObj.contains("detection") && rootObj["detection"].isObject()) {
        QJsonObject detectionObj = rootObj["detection"].toObject();
        
        // 检查检测功能是否在配置中启用
        if (detectionObj.contains("enabled") && detectionObj["enabled"].isBool()) {
          bool detectionConfigEnabled = detectionObj["enabled"].toBool();
          if (!detectionConfigEnabled) {
            qDebug() << "配置文件中检测功能已禁用，跳过检测初始化";
            return;
          }
        }
      }
    }
  }
  
  // 设置模型路径 - 需要根据实际路径调整
  QString modelPathDV = "./models/dv.pt";
  QString modelPathDVS = "./models/dvs.pt";
  
  qDebug() << "正在初始化检测功能...";
  qDebug() << "DV模型路径:" << modelPathDV;
  qDebug() << "DVS模型路径:" << modelPathDVS;
  
  // 检查模型文件是否存在
  QFileInfo dvModelFile(modelPathDV);
  QFileInfo dvsModelFile(modelPathDVS);
  
  if (!dvModelFile.exists()) {
    qDebug() << "警告: DV模型文件不存在:" << modelPathDV;
    qDebug() << "当前工作目录:" << QDir::currentPath();
    qDebug() << "将尝试使用默认路径或跳过DV检测功能";
  } else {
    qDebug() << "DV模型文件找到:" << dvModelFile.absoluteFilePath();
  }
  
  if (!dvsModelFile.exists()) {
    qDebug() << "警告: DVS模型文件不存在:" << modelPathDVS;
    qDebug() << "当前工作目录:" << QDir::currentPath();
    qDebug() << "将尝试使用默认路径或跳过DVS检测功能";
  } else {
    qDebug() << "DVS模型文件找到:" << dvsModelFile.absoluteFilePath();
  }
  
  try {
    // 获取检测模块实例
    dvDetector = DetectModule_DV::getInstance();
    dvsDetector = DetectModule_DVS::getInstance();
    resultManager = DualCameraResultManager::getInstance();
    
    if (!dvDetector) {
      qDebug() << "错误: 无法获取DV检测模块实例";
      return;
    }
    
    if (!dvsDetector) {
      qDebug() << "错误: 无法获取DVS检测模块实例";
      return;
    }
    
    if (!resultManager) {
      qDebug() << "错误: 无法获取结果管理器实例";
      return;
    }
    
    // 立即加载模型，并监听加载结果
    connect(dvDetector, &DetectModule_DV::modelLoadResult,
            this, &MainWindow::onModelLoadResult);
    connect(dvsDetector, &DetectModule_DVS::modelLoadResult,
            this, &MainWindow::onModelLoadResult);
    
    qDebug() << "开始加载检测模型...";
    bool dvModelLoaded = dvDetector->setModelPath(modelPathDV);
    bool dvsModelLoaded = dvsDetector->setModelPath(modelPathDVS);
    
    if (!dvModelLoaded) {
      qDebug() << "警告: DV模型加载失败";
    }
    if (!dvsModelLoaded) {
      qDebug() << "警告: DVS模型加载失败";
    }
    
    // 连接检测结果信号
    connect(dvDetector, &DetectModule_DV::newDetectResultImg, 
            this, &MainWindow::updateDVDetectionView);
    connect(dvDetector, &DetectModule_DV::newResult, 
            this, &MainWindow::onDVDetectionResult);
            
    connect(dvsDetector, &DetectModule_DVS::newDetectResultImg, 
            this, &MainWindow::updateDVSDetectionView);
    connect(dvsDetector, &DetectModule_DVS::newResult, 
            this, &MainWindow::onDVSDetectionResult);
    
    // 连接最终判决信号
    connect(resultManager, &DualCameraResultManager::finalDecisionMade,
            this, &MainWindow::onFinalDecision);
    
    // 初始化结果管理器
    resultManager->initialize("detection_results.txt");
    
    // 模型加载完成，但不启动推理线程，等待用户启用检测功能
    qDebug() << "检测功能初始化完成，模型已加载，等待用户启用检测功能";
    
  } catch (const std::exception& e) {
    qDebug() << "检测功能初始化异常:" << e.what();
  } catch (...) {
    qDebug() << "检测功能初始化未知异常";
  }
}

void MainWindow::onModelLoadResult(bool success, QString message) {
  qDebug() << "模型加载结果:" << success << message;
  if (!success) {
    // 模型加载失败时显示错误信息
    QMessageBox::warning(this, "模型加载失败", message);
  }
}

void MainWindow::onDetectionToggled(bool enabled) {
  qDebug() << "检测状态切换请求:" << (enabled ? "启用" : "禁用");

  // 统一的状态验证
  if (enabled) {
    // 检查检测模块是否已初始化
    if (!dvDetector || !dvsDetector || !resultManager) {
      qDebug() << "错误: 检测模块未初始化";
      QMessageBox::warning(this, "检测启动失败", "检测模块未初始化，请重启应用程序");
      enableDetectionCheckBox->setChecked(false);
      return;
    }

    // 检查模型是否已加载
    bool dvModelReady = dvDetector->isModelLoaded();
    bool dvsModelReady = dvsDetector->isModelLoaded();

    if (!dvModelReady || !dvsModelReady) {
      qDebug() << "警告: 模型未完全加载 DV:" << dvModelReady << " DVS:" << dvsModelReady;
      QMessageBox::warning(this, "检测启动失败",
                          QString("模型未完全加载\nDV模型: %1\nDVS模型: %2\n请检查模型文件")
                          .arg(dvModelReady ? "已加载" : "未加载")
                          .arg(dvsModelReady ? "已加载" : "未加载"));
      enableDetectionCheckBox->setChecked(false);
      return;
    }
  }

  // 更新状态变量
  detectionEnabled = enabled;
  enableDetect = enabled; // 更新全局变量

  // 统一的推理线程管理
  if (enabled) {
    qDebug() << "启用检测功能，启动推理线程";

    // 异步启动检测会话，避免阻塞UI
    QtConcurrent::run([this]() {
      auto sessionManager = DetectionSessionManager::getInstance();
      sessionManager->startDetectionSession();
    });

    bool dvStarted = false;
    bool dvsStarted = false;

    // 启动DV推理线程
    if (dvDetector && !dvDetector->isInferenceRunning()) {
      try {
        dvDetector->startInference();
        dvStarted = true;
        qDebug() << "DV检测推理线程已启动";
      } catch (const std::exception& e) {
        qDebug() << "DV检测推理线程启动失败:" << e.what();
        QMessageBox::warning(this, "检测启动失败", QString("DV推理线程启动失败: %1").arg(e.what()));
      }
    } else if (dvDetector && dvDetector->isInferenceRunning()) {
      dvStarted = true; // 已经在运行
    }

    // 启动DVS推理线程
    if (dvsDetector && !dvsDetector->isInferenceRunning()) {
      try {
        dvsDetector->startInference();
        dvsStarted = true;
        qDebug() << "DVS检测推理线程已启动";
      } catch (const std::exception& e) {
        qDebug() << "DVS检测推理线程启动失败:" << e.what();
        QMessageBox::warning(this, "检测启动失败", QString("DVS推理线程启动失败: %1").arg(e.what()));
      }
    } else if (dvsDetector && dvsDetector->isInferenceRunning()) {
      dvsStarted = true; // 已经在运行
    }

    // 如果任一推理线程启动失败，回滚状态
    if (!dvStarted || !dvsStarted) {
      qDebug() << "推理线程启动不完整，回滚状态";

      // 停止已启动的线程
      if (dvStarted && dvDetector && dvDetector->isInferenceRunning()) {
        try {
          dvDetector->stopInference();
        } catch (const std::exception& e) {
          qDebug() << "回滚时停止DV推理线程失败:" << e.what();
        }
      }
      if (dvsStarted && dvsDetector && dvsDetector->isInferenceRunning()) {
        try {
          dvsDetector->stopInference();
        } catch (const std::exception& e) {
          qDebug() << "回滚时停止DVS推理线程失败:" << e.what();
        }
      }

      // 恢复状态
      detectionEnabled = false;
      enableDetect = false;
      enableDetectionCheckBox->setChecked(false);
      updateDetectionStatusLabels(false);

      QMessageBox::warning(this, "检测启动失败", "推理线程启动不完整，检测功能已禁用");
      return;
    }
  } else {
    qDebug() << "禁用检测功能，停止推理线程";

    // 异步结束检测会话，避免阻塞UI
    QtConcurrent::run([this]() {
      auto sessionManager = DetectionSessionManager::getInstance();
      sessionManager->endDetectionSession();
    });

    bool dvStopped = true;
    bool dvsStopped = true;

    // 停止DV推理线程
    if (dvDetector && dvDetector->isInferenceRunning()) {
      try {
        dvDetector->stopInference();
        qDebug() << "DV检测推理线程已停止";
      } catch (const std::exception& e) {
        qDebug() << "DV检测推理线程停止失败:" << e.what();
        dvStopped = false;
      }
    }

    // 停止DVS推理线程
    if (dvsDetector && dvsDetector->isInferenceRunning()) {
      try {
        dvsDetector->stopInference();
        qDebug() << "DVS检测推理线程已停止";
      } catch (const std::exception& e) {
        qDebug() << "DVS检测推理线程停止失败:" << e.what();
        dvsStopped = false;
      }
    }

    // 如果停止失败，记录警告但不阻止状态切换
    if (!dvStopped || !dvsStopped) {
      qDebug() << "警告: 部分推理线程停止失败，但继续状态切换";
      QString warningMsg = "推理线程停止时出现问题:\n";
      if (!dvStopped) warningMsg += "- DV推理线程停止失败\n";
      if (!dvsStopped) warningMsg += "- DVS推理线程停止失败\n";
      warningMsg += "检测功能已禁用，但可能需要重启应用程序";

      QMessageBox::warning(this, "推理线程停止警告", warningMsg);
    }
  }

  // 统一的UI标签更新
  updateDetectionStatusLabels(enabled);

  qDebug() << "检测状态切换完成:" << (enabled ? "已启用" : "已禁用");
}

void MainWindow::resetDetectionSession() {
  qDebug() << "手动重置检测会话";

  // 异步重置检测会话，避免阻塞UI
  QtConcurrent::run([this]() {
    auto sessionManager = DetectionSessionManager::getInstance();
    sessionManager->resetDetectionSession();
    qDebug() << "检测会话重置完成";
  });
}

// 统一的检测状态标签更新方法
void MainWindow::updateDetectionStatusLabels(bool enabled) {
  if (enabled) {
    // 检测功能启用时，重置所有标签为等待状态
    dvDetectionResultLabel->setText(tr("DV检测结果: 等待中"));
    dvsDetectionResultLabel->setText(tr("DVS检测结果: 等待中"));
    finalDecisionLabel->setText(tr("最终判决: 等待中"));

    // 更新显示模式
    currentDisplayMode = DisplayMode::DETECTION_VIEW;
  } else {
    // 检测功能禁用时，显示禁用状态
    dvDetectionResultLabel->setText(tr("DV检测结果: 已禁用"));
    dvsDetectionResultLabel->setText(tr("DVS检测结果: 已禁用"));
    finalDecisionLabel->setText(tr("最终判决: 已禁用"));

    // 更新显示模式
    currentDisplayMode = DisplayMode::CAMERA_VIEW;
  }
}

void MainWindow::updateDVDetectionView(QImage img) {
  // 使用统一的检测结果显示管理器
  updateDetectionDisplay(img, "DV");
}

void MainWindow::updateDVSDetectionView(QImage img) {
  // 使用统一的检测结果显示管理器
  updateDetectionDisplay(img, "DVS");
}

void MainWindow::onDVDetectionResult(QString result) {
  // 确保在主线程中执行UI更新
  if (QThread::currentThread() != this->thread()) {
    QMetaObject::invokeMethod(this, "onDVDetectionResult",
                             Qt::QueuedConnection, Q_ARG(QString, result));
    return;
  }

  if (detectionEnabled) {
    dvDetectionResultLabel->setText(tr("DV检测结果: ") + result);

    // 解析检测结果并添加到结果管理器
    bool hasDetection = !result.contains("无");
    QString className = "";
    float confidence = 0.0f;

    if (hasDetection) {
      // 简单的结果解析，可根据实际格式调整
      QStringList parts = result.split(" ");
      if (parts.size() >= 2) {
        className = parts[1];
        confidence = 0.8f; // 默认置信度，可从结果中解析
      }

      // 如果配置了检测时自动录制，且当前没有在录制
      if (recordingWithDetection && hasDetection && !isMaxRecording && !isMidRecording && !isMinRecording) {
        qDebug() << "DV检测到目标，自动启动录制";
        auto path = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH_mm_ss") + "_dv_detect";
        emit this->startRecord(path);
      }
    }

    resultManager->addDVResult(result, hasDetection, className, confidence);
  }
}

void MainWindow::onDVSDetectionResult(QString result) {
  // 确保在主线程中执行UI更新
  if (QThread::currentThread() != this->thread()) {
    QMetaObject::invokeMethod(this, "onDVSDetectionResult",
                             Qt::QueuedConnection, Q_ARG(QString, result));
    return;
  }

  if (detectionEnabled) {
    dvsDetectionResultLabel->setText(tr("DVS检测结果: ") + result);

    // 解析检测结果并添加到结果管理器
    bool hasDetection = !result.contains("无");
    QString className = "";
    float confidence = 0.0f;

    if (hasDetection) {
      // 简单的结果解析，可根据实际格式调整
      QStringList parts = result.split(" ");
      if (parts.size() >= 2) {
        className = parts[1];
        confidence = 0.8f; // 默认置信度，可从结果中解析
      }

      // 如果配置了检测时自动录制，且当前没有在录制
      if (recordingWithDetection && hasDetection && !isMaxRecording && !isMidRecording && !isMinRecording) {
        qDebug() << "DVS检测到目标，自动启动录制";
        auto path = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH_mm_ss") + "_dvs_detect";
        emit this->startRecord(path);
      }
    }

    resultManager->addDVSResult(result, hasDetection, className, confidence);
  }
}

void MainWindow::onFinalDecision(QString decision, QDateTime timestamp) {
  // 确保在主线程中执行UI更新
  if (QThread::currentThread() != this->thread()) {
    QMetaObject::invokeMethod(this, "onFinalDecision",
                             Qt::QueuedConnection,
                             Q_ARG(QString, decision),
                             Q_ARG(QDateTime, timestamp));
    return;
  }

  if (detectionEnabled) {
    finalDecisionLabel->setText(tr("最终判决: ") + decision);
    qDebug() << "最终判决:" << decision << "时间:" << timestamp.toString();
  }
}

// 优化的检测结果图片保存功能 - 使用FileSaveManager异步保存
void MainWindow::saveDetectionResultImage(const QImage &img, const QString &cameraType) {
  // 只在检测启用且有有效图像时保存
  if (!detectionEnabled || img.isNull()) {
    return;
  }

  // 使用FileSaveManager的智能限流保存
  uint64_t timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();

  // 获取当前处理的原始文件名
  QString originalFilename;
  if (cameraType == "DV") {
    originalFilename = currentDVFilename;
  } else if (cameraType == "DVS") {
    originalFilename = currentDVSFilename;
  }

  FileSaveManager::getInstance()->saveDetectionImageWithThrottling(img, cameraType, timestamp, originalFilename);
}

// 加载主配置文件
void MainWindow::loadMainConfig(const QString &configPath) {
  QFile file(configPath);
  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() << "Could not open main configuration file:" << configPath;
    return;
  }

  QByteArray jsonData = file.readAll();
  QJsonDocument document = QJsonDocument::fromJson(jsonData);
  
  if (document.isNull() || !document.isObject()) {
    qDebug() << "Invalid JSON format in main configuration file:" << configPath;
    return;
  }

  QJsonObject rootObj = document.object();
  
  // 读取自动启动配置
  if (rootObj.contains("autoStart") && rootObj["autoStart"].isObject()) {
    QJsonObject autoStartObj = rootObj["autoStart"].toObject();
    
    if (autoStartObj.contains("camera") && autoStartObj["camera"].isBool()) {
      autoStartCamera = autoStartObj["camera"].toBool();
    }
    
    if (autoStartObj.contains("detection") && autoStartObj["detection"].isBool()) {
      autoStartDetection = autoStartObj["detection"].toBool();
    }
    
    if (autoStartObj.contains("recording") && autoStartObj["recording"].isBool()) {
      autoStartRecording = autoStartObj["recording"].toBool();
    }
    
    if (autoStartObj.contains("recordingWithDetection") && autoStartObj["recordingWithDetection"].isBool()) {
      recordingWithDetection = autoStartObj["recordingWithDetection"].toBool();
    }
  }
  
  qDebug() << "Loaded main configuration from" << configPath;
  qDebug() << "Auto start options:"
           << "\n  autoStartCamera:" << autoStartCamera
           << "\n  autoStartDetection:" << autoStartDetection
           << "\n  autoStartRecording:" << autoStartRecording
           << "\n  recordingWithDetection:" << recordingWithDetection;
  
  // 根据配置自动启动功能
  if (autoStartCamera) {
    // 自动启动相机 - 延迟启动以确保UI完全初始化
    QTimer::singleShot(1000, [this]() {
      qDebug() << "根据配置自动启动DVS相机...";
      auto sourceInstance = DVSDataSource::getInstance();
      sourceInstance->startCamera();
      
      // 如果配置了自动启动录制，在相机启动后启动录制
      if (autoStartRecording) {
        QTimer::singleShot(2000, [this]() {
          qDebug() << "根据配置自动启动录制...";
          auto path = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH_mm_ss") + "_auto";
          emit this->startRecord(path);
        });
      }
    });
  }
  
  // 如果配置了自动启动检测功能
  if (autoStartDetection) {
    // 延迟启动检测功能，确保相机先启动
    QTimer::singleShot(1500, [this]() {
      qDebug() << "根据配置自动启动检测功能...";
      enableDetectionCheckBox->setChecked(true);
      detectionEnabled = true;
      enableDetect = true;
      onDetectionToggled(true);
    });
  }
  
  // 如果配置了检测功能与录制联动
  if (recordingWithDetection) {
    qDebug() << "检测功能与录制联动已启用";
    // 检测结果处理逻辑已在onDVDetectionResult和onDVSDetectionResult中实现
  }
}

// 初始化Modbus连接
void MainWindow::initializeModbusConnection() {
  qDebug() << "=== 开始初始化Modbus连接 ===";
  
  // 创建带IP和端口的socket实例
  const char* ip = "192.168.1.100";  // 根据实际情况修改IP地址
  int port = 502;                    // Modbus TCP默认端口
  
  qDebug() << "连接参数 - IP:" << ip << "端口:" << port;
  
  try {
    // 创建socket实例并连接
    socket = new modbusSocket(const_cast<char*>(ip), port);
    socket->open();
    qDebug() << "Modbus连接初始化完成";
  } catch (const std::exception& e) {
    qDebug() << "Modbus连接初始化异常:" << e.what();
    // 如果连接失败，创建一个默认的socket实例
    socket = new modbusSocket();
    qDebug() << "使用默认socket实例";
  } catch (...) {
    qDebug() << "Modbus连接初始化未知异常";
    // 如果连接失败，创建一个默认的socket实例
    socket = new modbusSocket();
    qDebug() << "使用默认socket实例";
  }
  
  // 连接自动录制相关信号
  connect(socket, &modbusSocket::autoRecordingComplete, this, [this]() {
    qDebug() << "自动录制流程完成";
    QMessageBox::information(this, "自动录制完成", "25张盘片的自动录制流程已完成！");

    // 恢复按钮状态
    if (autoRecordingButton) {
      autoRecordingButton->setEnabled(true);
      autoRecordingButton->setText("自动录制模式");
    }
    if (autoRecordingNoCommButton) {
      autoRecordingNoCommButton->setEnabled(true);
      autoRecordingNoCommButton->setText("自动录制（无通讯）");
    }
  });
  
  connect(socket, &modbusSocket::discRecordingComplete, this, [this](int discNumber) {
    qDebug() << "第" << discNumber << "张盘片录制完成";

    // 更新按钮文本显示进度
    if (autoRecordingButton && !autoRecordingButton->isEnabled()) {
      autoRecordingButton->setText(QString("自动录制中... (%1/25)").arg(discNumber));
    }
    if (autoRecordingNoCommButton && !autoRecordingNoCommButton->isEnabled()) {
      autoRecordingNoCommButton->setText(QString("无通讯录制中... (%1/25)").arg(discNumber));
    }
  });
  
  // 连接socket的录制信号到相机录制功能
  connect(socket, &modbusSocket::startRecord, this, &MainWindow::startRecord);
  connect(socket, &modbusSocket::stopRecord, this, [this]() {
    emit this->stopRecord();
    emit this->saveRecord("");
  });

  qDebug() << "=== Modbus连接初始化结束 ===";
}
