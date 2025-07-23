//
// Created by pe on 2021/9/12.
//

#include "CameraControlButtonGroup.h"
#include "DVDataSaveFormatWidget.h"
#include "DVSelectWidget.h"
#include "DenoiseWidget.h"
#include "OverlapSelectWidget.h"
#include "SaveOptionsDialog.h"
#include "SpeedSelectWidget.h"
#include "ThresholdWidget.h"
#include "PlaybackWidget.h"
#include "camera/CameraConfig.h"
#include "dvs/DVSDataSource.h"
#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <opencv2/opencv.hpp>
#include <QDebug>

CameraControlButtonGroup::CameraControlButtonGroup(
    const CameraControlConfig &config, QWidget *parent)
    : QWidget(parent), playbackReader(PlaybackReader::getInstance()),
      dvsDataSource(DVSDataSource::getInstance()) {
  auto vboxLayout = new QVBoxLayout;
  auto cameraLayout = new QHBoxLayout();
  if (config.denoise) {
    auto denoiseModule = new DenoiseWidget;
    cameraLayout->addWidget(denoiseModule);
  }
  if (config.saveFormat) {
    auto saveImgFormatWidget = new DVDataSaveFormatWidget;
    cameraLayout->addWidget(saveImgFormatWidget);
    connect(
        this, &CameraControlButtonGroup::camerasStarted, this,
        [saveImgFormatWidget]() { saveImgFormatWidget->setDisabled(true); });
  }
  if (config.dv) {
    auto dvSelectWidget = new DVSelectWidget;
    cameraLayout->addWidget(dvSelectWidget);
  }
  if (config.speed) {
    auto speedWidget = new SpeedSelectWidget;
    cameraLayout->addWidget(speedWidget);

    auto syncImageButton = new QPushButton("同步DVS数据");
    connect(syncImageButton, &QPushButton::clicked, this, []() {
      auto ins = DVSDataSource::getInstance();
      ins->syncImage();
    });
    cameraLayout->addWidget(syncImageButton);
  }
  if (config.overlap) {
    auto overlapWidget = new OverlapSelectWidget;
    cameraLayout->addWidget(overlapWidget);
  }
  
  // 使用新的PlaybackWidget替代原来的按钮
  if (config.playback) {
    // 检查全局回放配置是否启用
    bool showPlaybackWidget = CameraConfig::getInstance().getPlaybackConfig().enabled;
    
    if (showPlaybackWidget) {
      auto playbackWidget = new PlaybackWidget();
      cameraLayout->addWidget(playbackWidget);
      
      // 连接信号
      connect(playbackWidget, &PlaybackWidget::playbackStarted, 
              this, &CameraControlButtonGroup::playbackStarted);
              
      // 相机启动时禁用回放按钮 - 使用lambda修复参数不匹配问题
      connect(this, &CameraControlButtonGroup::camerasStarted, 
              [playbackWidget]() { playbackWidget->setDisabled(true); });
    } else {
      qDebug() << "回放功能已在配置文件中禁用";
    }
  }

  //不显示
  if (config.modelSelect && false) {
    auto loadEngineFileButton = new QPushButton("载入模型文件");
    connect(loadEngineFileButton, &QPushButton::clicked, this,
            [loadEngineFileButton]() {
              auto file = QFileDialog::getOpenFileName();
              if (!file.isEmpty()) {
                loadEngineFileButton->setText("更改模型:" +
                                              file.split("/").last());
              }
            });
    cameraLayout->addWidget(loadEngineFileButton);
  }
  //不显示
  if (config.modelPolicySelect && false) {
    auto selectModel = new QComboBox();
    selectModel->addItem("8类模型", "EightModelPolicy");
    // selectModel->addItem("数字目标", 1);
    connect(selectModel, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [selectModel](int id) {
              CameraConfig::getInstance().setClassificationModelPolicy(
                  selectModel->itemData(id).toString());
            });
    selectModel->setCurrentIndex(0);
    CameraConfig::getInstance().setClassificationModelPolicy(
        selectModel->currentData().toString());
    cameraLayout->addWidget(selectModel);
  }
  if (config.saveOptions) {
    auto saveOptionsButton = new QPushButton(tr("保存选项"));
    cameraLayout->addWidget(saveOptionsButton);
    connect(saveOptionsButton, &QPushButton::clicked, this, []() {
      SaveOptionsDialog::SaveOptions options;
      auto dvsInstance = DVSDataSource::getInstance();
      
      // 添加调试信息
      qDebug() << "Current enableSaveRaw status:" << dvsInstance->getEnableSaveRaw();
      
      options.saveDVSRaw = dvsInstance->getEnableSaveRaw();
      auto cameraInstance = CameraCapture::getInstance();
      options.saveDV = cameraInstance->getAllowRecording();
      options.saveDVSImg = dvsInstance->getEnableSaveImg();
      
      // 添加保存路径选项
      options.savePath = dvsInstance->getSaveFolderPath();
      
      auto dialog = new SaveOptionsDialog(options);
      if (dialog->exec() == QDialog::Accepted) {
        options = dialog->getConfig();
        dvsInstance->setEnableSaveRaw(options.saveDVSRaw);
        dvsInstance->setEnableSaveImg(options.saveDVSImg);
        cameraInstance->setAllowRecording(options.saveDV);
        
        // 如果保存路径已更改，则更新所有相关模块
        if (!options.savePath.isEmpty() && options.savePath != dvsInstance->getSaveFolderPath()) {
          dvsInstance->setSaveFolderPath(options.savePath);
          cameraInstance->setSavePath(options.savePath);
          
          // 确保目录存在
          QDir().mkpath(options.savePath);
          qDebug() << "保存路径已更新为:" << options.savePath;
        }
        
        qDebug() << "Updated enableSaveRaw status:" << options.saveDVSRaw;
      }
      dialog->deleteLater();
    });
  }

  cameraLayout->setAlignment(Qt::AlignHCenter);
  vboxLayout->addLayout(cameraLayout);

  if (config.threshold) {
    auto thresholdWidget = new ThresholdWidget;
    vboxLayout->addWidget(thresholdWidget);
  }
  setLayout(vboxLayout);
}

void CameraControlButtonGroup::startCamera() {
  // 相机启动时发送信号
  emit camerasStarted();
}
