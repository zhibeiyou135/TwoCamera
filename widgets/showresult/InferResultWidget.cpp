#include "InferResultWidget.h"
#include "camera/CameraConfig.h"
#include "infer/classification/ClassificationModule.h"
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
InferResultWidget::InferResultWidget(QWidget *parent) : QWidget(parent) {
  auto label = new QLabel(tr("识别结果："));
  QFont font = label->font();
  font.setPointSize(CameraConfig::getInstance().getFontPointSize());
  label->setFont(font);
  // auto prefixLabel = new QLabel(tr("识别结果"));
  auto layout = new QHBoxLayout;
  // layout->addWidget(prefixLabel);
  layout->addWidget(label);
  setLayout(layout);
  connect(
      ClassificationModule::getInstance(), &ClassificationModule::result, this,
      [label](QString labels) { label->setText(tr("识别结果：") + labels); },
      Qt::ConnectionType::QueuedConnection);
}