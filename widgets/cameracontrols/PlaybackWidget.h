#ifndef PLAYBACKWIDGET_H
#define PLAYBACKWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>
#include "PlaybackReader.h"
#include "camera/CameraConfig.h"

class PlaybackWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit PlaybackWidget(QWidget *parent = nullptr);
    
signals:
    void playbackStarted();
    
private slots:
    void onPlaybackButtonClicked();
    void onSelectBinFileClicked();
    void updatePlaybackMode();
    
private:
    QPushButton *playbackButton;
    QPushButton *selectBinFileButton;
    QRadioButton *bothRadioButton;
    QRadioButton *dvsOnlyRadioButton;
    QRadioButton *dvOnlyRadioButton;
    
    QButtonGroup *modeButtonGroup;
    
    PlaybackReader *playbackReader;
    
    void setupUI();
    
    // 将bin文件可视化为图像
    void visualizeBinFile(const QString &binFilePath);
};

#endif // PLAYBACKWIDGET_H 