#ifndef INFERRESULTWIDGET_H
#define INFERRESULTWIDGET_H
#include <QWidget>
//展示分类识别结果的组件。结果从InferBase::inferResult信号中拿到
class InferResultWidget : public QWidget {
  Q_OBJECT
public:
  explicit InferResultWidget(QWidget *parent = nullptr);
};
#endif