//
// Created by pe on 2021/9/16.
//

#ifndef TWOCAMERA_DVDATASAVEFORMATWIDGET_H
#define TWOCAMERA_DVDATASAVEFORMATWIDGET_H

#include <QComboBox>
#include <QWidget>

class DVDataSaveFormatWidget : public QWidget {
  Q_OBJECT
public:
  explicit DVDataSaveFormatWidget(QWidget *parent = nullptr);

private:
  QComboBox *dvSaveImageFormat;
};

#endif // TWOCAMERA_DVDATASAVEFORMATWIDGET_H
