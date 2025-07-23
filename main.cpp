#include "mainwindow.h"

#include "camera/CameraConfig.h"
#include "camera/CameraCapture.h"
#include "camera/ConfigManager.h"
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QThreadPool>
#include <sys/stat.h>
#include <sys/types.h>
// #include "tools/BinVisualizer.h"
#include <QCoreApplication>

int main(int argc, char *argv[]) {
  umask(0);
  QThreadPool::globalInstance()->setMaxThreadCount(8);
  QApplication a(argc, argv);

  // 设置应用程序信息
  a.setApplicationName("TwoCamera");
  a.setApplicationVersion("1.0");
  a.setApplicationDisplayName("双相机检测系统");

  QCommandLineOption debugMode("d", "启用调试模式");
#ifndef BUILTIN_CONFIG
  QCommandLineOption configFilePath("c", "指定配置文件路径", "配置文件路径", "dv_dvs.json");
#endif

  QCommandLineParser parser;
  parser.setApplicationDescription("双相机检测系统 - 支持DV和DVS相机的图像采集、录制和检测功能");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
  parser.addOption(debugMode);
#ifndef BUILTIN_CONFIG
  parser.addOption(configFilePath);
#endif
  parser.process(a);

  auto &instance = CameraConfig::getInstance();
#ifndef BUILTIN_CONFIG
  // 获取配置文件路径并设置到ConfigManager
  QString configPath = parser.value(configFilePath);
  ConfigManager::getInstance().setConfigPath(configPath);
  
  qDebug() << "Using config file:" << configPath;
  
  // 加载配置文件，该文件同时包含主配置和DV显示配置
  instance.loadConfigFile(configPath);
  
  // 使用相同的配置文件加载DV显示配置
  CameraCapture::getInstance()->loadConfig(configPath);
#else
  // 即使在BUILTIN_CONFIG模式下也设置ConfigManager
  ConfigManager::getInstance().setConfigPath("dv_dvs.json");
  instance.loadConfigFile("dv_dvs.json");
  CameraCapture::getInstance()->loadConfig("dv_dvs.json");
#endif

  if (parser.isSet(debugMode)) {
    qDebug() << "调试";
    instance.setDebugMode(true);
  }
  MainWindow w;
  w.setWindowTitle("演示程序");
  w.show();

  // BinVisualizer visualizer;
  
  // // 设置输入bin文件路径和输出目录
  // QString binPath = "path/to/your/dvs.bin";
  // QString outputDir = "path/to/output/directory";
  
  // if (visualizer.convertBinToImages(binPath, outputDir)) {
  //   qDebug() << "Successfully converted bin file to images";
  // } else {
  //   qDebug() << "Failed to convert bin file";
  // }

  return a.exec();
}
