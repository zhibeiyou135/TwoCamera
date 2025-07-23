//
// Created by pe on 2021/9/16.
//

#include "ThresholdWidget.h"
#include "dvs/DVSDataSource.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>

ThresholdWidget::ThresholdWidget(QWidget *parent) : QWidget(parent) {
  auto layout = new QHBoxLayout;
  auto label = new QLabel(tr("阈值:"));
  layout->addWidget(label);
  auto slider = new QSlider(Qt::Horizontal);
  slider->setMaximum(255);
  slider->setMinimum(1);
  slider->setValue(171);
  layout->addWidget(slider);
  auto spinBox = new QSpinBox;
  spinBox->setMaximum(255);
  spinBox->setMinimum(1);
  spinBox->setValue(171);
  layout->addWidget(spinBox);
  connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), slider,
          &QSlider::setValue);
  connect(slider, &QSlider::valueChanged, spinBox, &QSpinBox::setValue);
  connect(slider, &QSlider::valueChanged, this, [](int value) {
    // Note: Threshold setting is now handled through DVS bias configuration
    // DVSDataSource::getInstance()->setThreshold(value);
    qDebug() << "Threshold value changed to:" << value;
  });
  setLayout(layout);
}
