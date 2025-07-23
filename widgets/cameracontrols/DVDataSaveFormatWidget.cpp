//
// Created by pe on 2021/9/16.
//

#include "DVDataSaveFormatWidget.h"
#include "camera/CameraConfig.h"
#include <QHBoxLayout>
#include <QLabel>

DVDataSaveFormatWidget::DVDataSaveFormatWidget(QWidget *parent)
    : QWidget(parent) {
  auto cameraLayout = new QHBoxLayout;
  auto saveImgFormatLabel = new QLabel(tr("图像保存格式："));
  cameraLayout->addWidget(saveImgFormatLabel);
  dvSaveImageFormat = new QComboBox();
  dvSaveImageFormat->addItem(tr("BMP格式"), ".bmp");
  dvSaveImageFormat->addItem(tr("JPG格式"), ".jpg");
  connect(dvSaveImageFormat,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          [this](int index) {
            CameraConfig::getInstance().dvImageSaveFormat =
                dvSaveImageFormat->itemData(index).toString().toStdString();
          });
  dvSaveImageFormat->setCurrentIndex(0);
  cameraLayout->addWidget(dvSaveImageFormat);
  setLayout(cameraLayout);
}
