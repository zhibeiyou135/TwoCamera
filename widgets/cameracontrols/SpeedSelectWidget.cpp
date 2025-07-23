//
// Created by pe on 2021/9/17.
//

#include "SpeedSelectWidget.h"
#include "dvs/DVSDataSource.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>

SpeedSelectWidget::SpeedSelectWidget(QWidget *parent) : QWidget(parent) {
  auto layout = new QHBoxLayout;
  auto label = new QLabel(tr("播放倍率"));
  auto comboBox = new QComboBox;

  comboBox->addItem("1", 1);
  comboBox->addItem("2", 2);
  comboBox->addItem("3", 3);
  comboBox->addItem("5", 5);
  //    comboBox->addItem("7", 7);
  comboBox->addItem("10", 10);
  //    comboBox->addItem("15", 15);
  //    comboBox->addItem("20", 20);
      comboBox->addItem("25", 25);
      comboBox->addItem("100", 100);
  //    comboBox->addItem("150", 150);
  //    comboBox->addItem("200", 200);

  connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          [comboBox](int c) {
            auto instance = DVSDataSource::getInstance();
            instance->setSpeed(comboBox->itemData(c).toInt());
          });

  layout->addWidget(label);
  layout->addWidget(comboBox);
  setLayout(layout);
}
