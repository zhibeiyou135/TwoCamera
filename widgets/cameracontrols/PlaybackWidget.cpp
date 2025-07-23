#include "PlaybackWidget.h"
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QDir>
#include <QDateTime>
#include <fstream>
#include <vector>

PlaybackWidget::PlaybackWidget(QWidget *parent)
    : QWidget(parent), playbackReader(PlaybackReader::getInstance())
{
    setupUI();
    
    // 初始化回放模式（从配置中读取）
    auto &playbackConfig = CameraConfig::getInstance().getPlaybackConfig();
    
    if (playbackConfig.mode == "both") {
        bothRadioButton->setChecked(true);
    } else if (playbackConfig.mode == "dvs") {
        dvsOnlyRadioButton->setChecked(true);
    } else if (playbackConfig.mode == "dv") {
        dvOnlyRadioButton->setChecked(true);
    }
    
    // 连接回放完成信号，使回放按钮可用
    connect(playbackReader, &PlaybackReader::complete, this,
            [this]() { 
                playbackButton->setEnabled(true); 
                selectBinFileButton->setEnabled(true);
            });
}

void PlaybackWidget::setupUI()
{
    auto mainLayout = new QVBoxLayout(this);
    
    // 创建模式选择框
    auto modeGroupBox = new QGroupBox(tr("回放模式"));
    auto modeLayout = new QVBoxLayout();
    
    // 创建单选按钮
    bothRadioButton = new QRadioButton(tr("DVS与DV同时回放"));
    dvsOnlyRadioButton = new QRadioButton(tr("仅回放DVS"));
    dvOnlyRadioButton = new QRadioButton(tr("仅回放DV"));
    
    // 创建按钮组
    modeButtonGroup = new QButtonGroup(this);
    modeButtonGroup->addButton(bothRadioButton, 0);
    modeButtonGroup->addButton(dvsOnlyRadioButton, 1);
    modeButtonGroup->addButton(dvOnlyRadioButton, 2);
    
    // 添加到布局
    modeLayout->addWidget(bothRadioButton);
    modeLayout->addWidget(dvsOnlyRadioButton);
    modeLayout->addWidget(dvOnlyRadioButton);
    modeGroupBox->setLayout(modeLayout);
    
    // 创建回放按钮
    auto buttonLayout = new QHBoxLayout();
    playbackButton = new QPushButton(tr("选择回放文件夹"));
    selectBinFileButton = new QPushButton(tr("选择Bin文件"));
    
    buttonLayout->addWidget(playbackButton);
    buttonLayout->addWidget(selectBinFileButton);
    
    // 添加到主布局
    mainLayout->addWidget(modeGroupBox);
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号槽
    connect(playbackButton, &QPushButton::clicked, this, &PlaybackWidget::onPlaybackButtonClicked);
    connect(selectBinFileButton, &QPushButton::clicked, this, &PlaybackWidget::onSelectBinFileClicked);
    connect(modeButtonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), 
            this, &PlaybackWidget::updatePlaybackMode);
}

void PlaybackWidget::onPlaybackButtonClicked()
{
    // 获取回放文件夹
    auto path = QFileDialog::getExistingDirectory(this, tr("选择回放文件夹"));
    if (path.isEmpty()) {
        return;
    }
    
    // 设置按钮不可用，防止重复点击
    playbackButton->setEnabled(false);
    selectBinFileButton->setEnabled(false);
    
    qDebug() << "选择回放文件夹:" << path;
    
    // 获取当前回放模式设置
    auto &playbackConfig = CameraConfig::getInstance().getPlaybackConfig();
    
    // 配置回放参数
    PlaybackReader::PlaybackParams params;
    params.path = path;
    params.dvEnabled = playbackConfig.dvEnabled;
    params.dvsEnabled = playbackConfig.dvsEnabled;
    
    // 启动回放
    this->playbackReader->startPlayback(params);
    
    // 发出回放开始信号
    emit playbackStarted();
}

void PlaybackWidget::onSelectBinFileClicked()
{
    // 获取bin文件路径
    auto filePath = QFileDialog::getOpenFileName(this, tr("选择DVS Bin文件"), 
                                               QString(), tr("Bin Files (*.bin)"));
    if (filePath.isEmpty()) {
        return;
    }
    
    // 设置按钮不可用，防止重复点击
    playbackButton->setEnabled(false);
    selectBinFileButton->setEnabled(false);
    
    qDebug() << "选择Bin文件:" << filePath;
    
    // 创建临时文件夹用于回放
    QString tempDir = QDir::tempPath() + "/dvs_playback_" + 
                     QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QDir().mkpath(tempDir);
    
    // 配置回放参数 - 只支持RAW文件
    PlaybackReader::PlaybackParams params;
    params.path = tempDir;
    params.dvEnabled = false;
    params.dvsEnabled = true;
    params.rawFilePath = filePath;
    params.visualizeOnly = true;
    
    // 启动回放
    this->playbackReader->startPlayback(params);
    
    // 发出回放开始信号
    emit playbackStarted();
}

void PlaybackWidget::updatePlaybackMode()
{
    auto &playbackConfig = CameraConfig::getInstance().getPlaybackConfig();
    
    // 根据选中的按钮设置回放模式
    if (bothRadioButton->isChecked()) {
        playbackConfig.mode = "both";
        playbackConfig.dvEnabled = true;
        playbackConfig.dvsEnabled = true;
    } else if (dvsOnlyRadioButton->isChecked()) {
        playbackConfig.mode = "dvs";
        playbackConfig.dvEnabled = false;
        playbackConfig.dvsEnabled = true;
    } else if (dvOnlyRadioButton->isChecked()) {
        playbackConfig.mode = "dv";
        playbackConfig.dvEnabled = true;
        playbackConfig.dvsEnabled = false;
    }
    
    qDebug() << "回放模式已更新为:" << playbackConfig.mode;
}

void PlaybackWidget::visualizeBinFile(const QString &binFilePath)
{
    qDebug() << "BIN文件不再支持，只支持RAW文件:" << binFilePath;

    // 通知用户BIN文件不再支持
    QMessageBox::warning(this, tr("不支持的文件格式"),
                        tr("BIN文件格式已不再支持，请使用RAW文件格式。"));
}