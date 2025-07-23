#ifndef IMAGERESULTWIDGET_H
#define IMAGERESULTWIDGET_H
#include <QLabel>
#include <QWidget>
//展示图片结果的组件，中间包括几个子组件，包括dv组件，dvs组件，dvs重构组件，dvs检测组件
class ImageResultWidget : public QWidget {
  Q_OBJECT
private:
  QLabel *dvView, *dvsView;

public:
  ImageResultWidget();
public slots:
  // void updateDVImage(QDImage img);
  // void updateDVSImage(QImage img);
};
#endif