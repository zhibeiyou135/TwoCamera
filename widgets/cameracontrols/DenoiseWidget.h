//
// Created by pe on 2021/9/16.
//

#ifndef TWOCAMERA_DENOISEWIDGET_H
#define TWOCAMERA_DENOISEWIDGET_H

#include "dvs/DVSDataSource.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

class DenoiseWidget : public QWidget {
  Q_OBJECT
public:
  explicit DenoiseWidget(QWidget *parent = nullptr) : QWidget(parent) {
    auto denoiseLabel = new QLabel(tr("去躁："));
    auto cameraLayout = new QHBoxLayout();
    cameraLayout->addWidget(denoiseLabel);
    auto denoiseComboBox = new QComboBox();
    denoiseComboBox->addItem(tr("否"), false);
    denoiseComboBox->addItem(tr("是"), true);
    denoiseComboBox->setCurrentIndex(0);
    cameraLayout->addWidget(denoiseComboBox);
    connect(denoiseComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [denoiseComboBox](int id) {
              DVSDataSource::getInstance()->setDenoise(
                  denoiseComboBox->itemData(id).toBool());
            });
    setLayout(cameraLayout);
  }
};

#endif // TWOCAMERA_DENOISEWIDGET_H
