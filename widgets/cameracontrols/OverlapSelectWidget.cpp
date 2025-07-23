//
// Created by pe on 2021/9/17.
//

#include "OverlapSelectWidget.h"
#include "dvs/DVSDataSource.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>

OverlapSelectWidget::OverlapSelectWidget(QWidget *parent) : QWidget(parent) {
  auto layout = new QHBoxLayout;
  auto label = new QLabel(tr("Overlap"));
  auto comboBox = new QComboBox;

  comboBox->addItem("1", 1);
  comboBox->addItem("2", 2);
  comboBox->addItem("3", 3);
  comboBox->addItem("4", 4);
  comboBox->addItem("5", 5);
  comboBox->addItem("6", 6);
  comboBox->addItem("7", 7);
  comboBox->addItem("10", 10);
  comboBox->addItem("20", 20);

  connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          [comboBox](int c) {
            auto instance = DVSDataSource::getInstance();
            instance->setOverlapCount(comboBox->itemData(c).toInt());
          });

  layout->addWidget(label);
  layout->addWidget(comboBox);
  setLayout(layout);
}
