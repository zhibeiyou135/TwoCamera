//
// Created by pe on 2021/9/12.
//

#ifndef TWOCAMERA_CAMERACONTROLBUTTONGROUP_H
#define TWOCAMERA_CAMERACONTROLBUTTONGROUP_H

#include "PlaybackReader.h"
#include "camera/CameraCapture.h"
#include "dvs/DVSDataSource.h"
#include <QComboBox>
#include <QPushButton>
#include <QWidget>

class CameraControlButtonGroup : public QWidget {
  Q_OBJECT
public:
  explicit CameraControlButtonGroup(const CameraControlConfig &config,
                                    QWidget *parent = nullptr);
signals:

  void dvStarted();

  void dvsStarted();

  void playbackStarted();

  void camerasStarted();

public slots:

  void startCamera();

private:
  PlaybackReader *playbackReader;
  DVSDataSource *dvsDataSource;
  CameraCapture *cameraCapture;
};

#endif // TWOCAMERA_CAMERACONTROLBUTTONGROUP_H
