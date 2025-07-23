//
// Created by pe on 2021/9/16.
//

#include "DVSelectWidget.h"
#include "camera/CameraCapture.h"
#include "camera/CamerasDetector.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <opencv2/opencv.hpp>

DVSelectWidget::DVSelectWidget(QWidget *parent) : QWidget(parent) {
  auto cameraLayout = new QHBoxLayout;
  auto cameraLabel = new QLabel(tr("DV相机数据源:"));
  cameraLayout->addWidget(cameraLabel);
  auto cameraComboBox = new QComboBox();
  QSizePolicy policy;
  policy.setVerticalPolicy(QSizePolicy::Policy::Fixed);
  policy.setHorizontalPolicy(QSizePolicy::Policy::Fixed);
  cameraComboBox->setSizePolicy(policy);
  const auto &camerasIndex = CamerasDetector::getInstance()->getCamerasIndex();
  for (auto i : camerasIndex) {
    // qDebug() << i;
    cameraComboBox->addItem(QString("%1").arg(i), QVariant::fromValue(i));
  }
  connect(cameraComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, [cameraComboBox](int id) {
            auto dvCapture = CameraCapture::getInstance();
            dvCapture->setCameraIndex(cameraComboBox->itemData(id).toInt());
          });
  cameraLayout->addWidget(cameraComboBox);
  cameraLayout->setAlignment(cameraComboBox, Qt::AlignLeft);
  setLayout(cameraLayout);
}
