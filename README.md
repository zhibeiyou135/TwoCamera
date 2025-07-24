# TwoCamera


# 回放功能完整流程详解

## 1. 回放功能架构概览

回放功能主要由 `PlaybackReader` 类实现，支持三种回放模式：
- **单文件回放**：播放单个RAW文件
- **同步回放**：同时播放DV图像和DVS RAW文件
- **批量回放**：递归播放多个会话文件夹

## 2. 用户交互入口

### 2.1 UI触发点
```cpp
// PlaybackWidget.cpp - 用户选择回放文件夹
void PlaybackWidget::onPlaybackButtonClicked() {
    auto path = QFileDialog::getExistingDirectory(this, tr("选择回放文件夹"));
    
    // 配置回放参数
    PlaybackReader::PlaybackParams params;
    params.path = path;
    params.dvEnabled = playbackConfig.dvEnabled;
    params.dvsEnabled = playbackConfig.dvsEnabled;
    
    // 启动回放
    this->playbackReader->startPlayback(params);
}
```

### 2.2 单文件RAW回放
```cpp
// mainwindow.cpp - 选择单个RAW文件回放
connect(selectRecordButton, &QPushButton::clicked, this, [this]() {
    QString recordFile = QFileDialog::getOpenFileName(this, tr("选择录制文件"), 
                                                    "", tr("录制文件 (*.raw *.bin)"));
    
    PlaybackReader::PlaybackParams params;
    params.visualizeOnly = true;
    params.rawFilePath = recordFile;
    
    PlaybackReader::getInstance()->visualizeRawFile(params);
});
```

## 3. 回放模式判断与分发

### 3.1 主要分发逻辑
```cpp
// PlaybackReader.cpp
void PlaybackReader::startPlayback(const QString &path) {
    loadFPN();  // 加载固定模式噪声校正
    
    QDir d(path);
    auto files = d.entryInfoList();
    QList<QFileInfo> dvImgs;
    QFileInfo dvsFileInfo;
    
    // 筛选数据文件
    for (auto file : files) {
        if (file.isFile()) {
            if ((file.fileName().endsWith(".jpg") ||
                file.fileName().endsWith(".bmp") ||
                file.fileName().endsWith(".png")) && dvEnabled) {
                dvImgs.push_back(file);
            } else if (file.fileName().endsWith(".raw") && dvsEnabled) {
                dvsFileInfo = file;
            }
        }
    }
    
    // 决定回放模式
    bool hasDVS = dvsEnabled && dvsFileInfo.exists();
    bool hasDV = dvEnabled && !dvImgs.empty();
    
    if (hasDVS && !hasDV) {
        // 只有DVS - 纯RAW文件回放
        visualizeRawFile(params);
    } else if (hasDV && !hasDVS) {
        // 只有DV - 图像序列回放
        playDVwithoutDVS(dvImgs);
    } else if (hasDVS && hasDV) {
        // 同时有DVS和DV - 同步回放
        playRAWWithDVImages(dvsFileInfo.absoluteFilePath(), dvImgs);
    }
}
```

## 4. 三种回放模式详解

### 4.1 纯DVS RAW文件回放

```cpp
void PlaybackReader::visualizeRawFile(const PlaybackParams &params) {
    auto camera = Metavision::Camera::from_file(rawFilePath.toStdString());
    
    // 创建帧生成算法
    int width = 1280, height = 720;
    const uint32_t accumulation_time = 10000;  // 10ms积累时间
    const double fps = 25.0;
    
    auto frame_generator = Metavision::PeriodicFrameGenerationAlgorithm(
        width, height, accumulation_time, fps);
    
    // 设置帧输出回调
    frame_generator.set_output_callback([&](Metavision::timestamp ts, cv::Mat &frame) {
        QImage frame_image = QImage(frame.data, frame.cols, frame.rows, 
                                  frame.step, QImage::Format_RGB888).copy();
        
        // 发送UI更新
        emit nextImagePair(dvImage, frame_image);  // dvImage为空白占位符
        
        QThread::msleep(40);  // 约25fps的播放速度
    });
    
    // 添加事件处理回调
    camera.cd().add_callback([&](const Metavision::EventCD *begin, const Metavision::EventCD *end) {
        frame_generator.process_events(begin, end);
    });
    
    camera.start();
    
    // 等待处理完成
    while (camera.is_running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
```

### 4.2 纯DV图像序列回放

```cpp
void PlaybackReader::playDVwithoutDVS(QList<QFileInfo> dvImgs) {
    if (dvImgs.empty()) {
        emit complete();
        return;
    }
    
    // 创建空白DVS图像作为占位符
    QImage dvsImg(1280, 720, QImage::Format_RGB888);
    dvsImg.fill(Qt::black);
    
    // 获取当前DV图像
    QFileInfo currentImgInfo = dvImgs.front();
    QImage dvImg = loadAndProcessDVImage(currentImgInfo.absoluteFilePath());
    
    // 提取时间戳（处理_cropped后缀）
    QString baseName = currentImgInfo.baseName();
    if (baseName.endsWith("_cropped")) {
        baseName = baseName.left(baseName.length() - 8);
    }
    auto lastTimestamp = baseName.toLongLong();
    
    dvImgs.pop_front();
    
    // 发送图像对
    emit nextImagePair(dvImg, dvsImg);
    
    if (dvImgs.empty()) {
        handlePlaybackFinished();
        emit complete();
        return;
    }
    
    // 计算下一帧延迟
    QFileInfo nextImgInfo = dvImgs.front();
    QString nextBaseName = nextImgInfo.baseName();
    if (nextBaseName.endsWith("_cropped")) {
        nextBaseName = nextBaseName.left(nextBaseName.length() - 8);
    }
    auto nextTimestamp = nextBaseName.toLongLong();
    
    // 基于时间戳差异计算延迟，最少30ms
    int delay = std::max(30, static_cast<int>((nextTimestamp - lastTimestamp) / 1000));
    
    // 调度下一帧
    QMetaObject::invokeMethod(this, [this, dvImgs, delay]() {
        QTimer::singleShot(delay, this, [this, dvImgs]() {
            playDVwithoutDVS(dvImgs);
        });
    }, Qt::QueuedConnection);
}
```

### 4.3 DV与DVS同步回放（核心功能）

这是最复杂的回放模式，需要精确的时间同步：

```cpp
void PlaybackReader::playRAWWithDVImages(const QString &rawFilePath, QList<QFileInfo> dvImgs) {
    // 1. 对DV图像按时间戳排序
    std::sort(dvImgs.begin(), dvImgs.end(), [](const QFileInfo &a, const QFileInfo &b) {
        QString aBaseName = a.baseName();
        QString bBaseName = b.baseName();
        if (aBaseName.endsWith("_cropped")) {
            aBaseName = aBaseName.left(aBaseName.length() - 8);
        }
        if (bBaseName.endsWith("_cropped")) {
            bBaseName = bBaseName.left(bBaseName.length() - 8);
        }
        return aBaseName.toLongLong() < bBaseName.toLongLong();
    });
    
    // 2. 计算DV时间范围
    uint64_t firstDVTimestamp = extractTimestamp(dvImgs.first());
    uint64_t lastDVTimestamp = extractTimestamp(dvImgs.last());
    uint64_t dvDuration = lastDVTimestamp - firstDVTimestamp;
    
    // 3. 预扫描RAW文件获取DVS时间范围
    uint64_t dvsStartTimestamp = 0;
    uint64_t dvsEndTimestamp = 0;
    prescanRawFile(rawFilePath, dvsStartTimestamp, dvsEndTimestamp);
    
    // 4. 计算时间缩放因子
    uint64_t dvsDuration = dvsEndTimestamp - dvsStartTimestamp;
    double timeScaleFactor = (dvsDuration > 0) ? 
        static_cast<double>(dvDuration) / dvsDuration : 1.0;
    
    // 5. 主播放循环
    auto camera = Metavision::Camera::from_file(rawFilePath.toStdString());
    auto frame_generator = Metavision::PeriodicFrameGenerationAlgorithm(1280, 720, 10000, 30.0);
    
    int dvImageIndex = 0;
    QImage currentDVImage = loadAndProcessDVImage(dvImgs[0].absoluteFilePath());
    
    frame_generator.set_output_callback([&](Metavision::timestamp ts, cv::Mat &frame) {
        // 6. 时间戳映射
        uint64_t dvsTimestamp = static_cast<uint64_t>(ts);
        uint64_t dvsRelativeTime = dvsTimestamp - dvsStartTimestamp;
        uint64_t mappedDVTimestamp = firstDVTimestamp + 
            static_cast<uint64_t>(dvsRelativeTime * timeScaleFactor);
        
        // 7. 查找最匹配的DV图像
        int targetIndex = findClosestDVImage(dvImgs, mappedDVTimestamp);
        if (targetIndex >= 0 && targetIndex != dvImageIndex) {
            dvImageIndex = targetIndex;
            currentDVImage = loadAndProcessDVImage(dvImgs[dvImageIndex].absoluteFilePath());
        }
        
        // 8. 处理DVS帧并发送
        cv::Mat resizedFrame;
        cv::resize(frame, resizedFrame, cv::Size(1280, 720));
        
        cv::Mat rgbFrame;
        cv::cvtColor(resizedFrame, rgbFrame, cv::COLOR_GRAY2RGB);
        
        QImage dvsImage(rgbFrame.data, rgbFrame.cols, rgbFrame.rows,
                       rgbFrame.step, QImage::Format_RGB888);
        
        emit nextImagePair(currentDVImage, dvsImage.copy());
        
        // 9. 更新进度
        double progress = (dvImageIndex + 1.0) / dvImgs.size();
        emit playbackProgress(std::min(progress, 1.0));
    });
    
    // 启动播放
    camera.start();
    while (running.load() && camera.is_running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    camera.stop();
}
```

## 5. 批量回放流程

### 5.1 会话收集
```cpp
void PlaybackReader::startBatchPlayback(const QString &rootDir) {
    // 停止当前播放
    stopPlayback();
    stopBatchPlayback();
    
    // 递归收集所有会话文件夹
    m_sessionQueue = collectSessionFolders(rootDir);
    
    if (m_sessionQueue.isEmpty()) {
        emit batchPlaybackFinished();
        return;
    }
    
    // 初始化批量播放状态
    m_currentSessionIndex = 0;
    m_batchPlaybackRunning.store(true);
    
    emit batchPlaybackStarted(m_sessionQueue.size());
    
    // 开始播放第一个会话
    playNextSession();
}

QList<SessionInfo> PlaybackReader::collectSessionFolders(const QString &rootDir) {
    QList<SessionInfo> sessionList;
    QDir dir(rootDir);
    
    // 检查当前目录是否为有效会话
    if (isValidSession(rootDir)) {
        SessionInfo session;
        session.sessionName = dir.dirName();
        session.dvPath = rootDir + "/dv_original";
        session.dvsPath = rootDir + "/dvs_raw";
        if (!QDir(session.dvsPath).exists()) {
            session.dvsPath = rootDir + "/dvs_images";
        }
        sessionList.append(session);
    }
    
    // 递归搜索子目录
    for (const auto& subDir : dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        auto subSessions = collectSessionFolders(subDir.absoluteFilePath());
        sessionList.append(subSessions);
    }
    
    return sessionList;
}
```

### 5.2 会话切换机制
```cpp
void PlaybackReader::playNextSession() {
    if (!m_batchPlaybackRunning.load() || m_currentSessionIndex >= m_sessionQueue.size()) {
        m_batchPlaybackRunning.store(false);
        emit batchPlaybackFinished();
        return;
    }
    
    const SessionInfo& session = m_sessionQueue[m_currentSessionIndex];
    
    // 发出进度信号
    emit batchPlaybackProgress(m_currentSessionIndex + 1, m_sessionQueue.size(), session.sessionName);
    
    // 查找文件
    QString dvsFile = findDVSFile(session.dvsPath);
    QList<QFileInfo> dvImages = loadDVImages(session.dvPath);
    
    if (dvsFile.isEmpty() || dvImages.isEmpty()) {
        // 跳到下一个会话
        m_currentSessionIndex++;
        QMetaObject::invokeMethod(this, [this]() {
            playNextSession();
        }, Qt::QueuedConnection);
        return;
    }
    
    // 确保停止当前播放
    stopPlayback();
    if (m_playbackThread.joinable()) {
        m_playbackThread.join();
    }
    
    // 启动新会话播放
    setFilePath(dvsFile);
    setDVFolderPath(session.dvPath);
    running.store(true);
    m_playbackThread = std::thread(&PlaybackReader::playRAWWithDVImages, this, dvsFile, dvImages);
}

void PlaybackReader::handlePlaybackFinished() {
    running.store(false);
    emit playbackFinished();
    
    // 批量播放自动切换
    if (m_batchPlaybackRunning.load()) {
        m_currentSessionIndex++;
        
        if (m_currentSessionIndex < m_sessionQueue.size()) {
            QMetaObject::invokeMethod(this, [this]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                playNextSession();
            }, Qt::QueuedConnection);
        } else {
            m_batchPlaybackRunning.store(false);
            emit batchPlaybackFinished();
        }
    }
}
```

## 6. UI显示与检测集成

### 6.1 图像显示流程
```cpp
// mainwindow.cpp
void MainWindow::onPlaybackImagePair(QImage dv, QImage dvs) {
    // 临时切换到相机视图模式
    DisplayMode originalMode = currentDisplayMode;
    currentDisplayMode = DisplayMode::CAMERA_VIEW;
    
    // 处理DV图像
    if (!dv.isNull()) {
        QMetaObject::invokeMethod(this, "updateCameraDisplay",
                                 Qt::QueuedConnection,
                                 Q_ARG(QImage, dv),
                                 Q_ARG(QString, "DV"));
        
        // 检测处理
        if (detectionEnabled) {
            QMetaObject::invokeMethod(this, "processDVImageForDetection",
                                     Qt::QueuedConnection, Q_ARG(QImage, dv));
        }
    }
    
    // 处理DVS图像
    if (!dvs.isNull()) {
        QMetaObject::invokeMethod(this, "updateCameraDisplay",
                                 Qt::QueuedConnection,
                                 Q_ARG(QImage, dvs),
                                 Q_ARG(QString, "DVS"));
        
        // 检测处理
        if (detectionEnabled) {
            QMetaObject::invokeMethod(this, "processDVSImageForDetection",
                                     Qt::QueuedConnection, Q_ARG(QImage, dvs));
        }
    }
    
    // 恢复原始显示模式
    currentDisplayMode = originalMode;
}
```

### 6.2 检测流程集成
回放模式下的检测流程与实时相机检测完全一致：
1. 图像格式转换（RGB888 → BGR for OpenCV）
2. 入队到对应的检测模块
3. 检测结果通过信号返回UI显示
4. 最终判决逻辑保持一致

## 7. 线程安全与资源管理

### 7.1 线程模型
- **主线程**：UI更新、信号槽处理
- **回放线程**：`std::thread` 执行实际的文件读取和处理
- **检测线程**：检测模块内部的推理线程

### 7.2 同步机制
- `std::atomic<bool> running`：控制回放循环
- `QMetaObject::invokeMethod`：跨线程调用
- `Qt::QueuedConnection`：确保UI更新在主线程

### 7.3 资源清理
```cpp
PlaybackReader::~PlaybackReader() {
    if (running.load()) {
        stopPlayback();
    }
}

void PlaybackReader::stopPlayback() {
    running.store(false);
    if (m_playbackThread.joinable()) {
        m_playbackThread.join();
    }
}
```

## 8. 支持的文件格式与目录结构

### 8.1 支持的文件格式
- **DVS文件**：`.raw` 格式（Metavision SDK）
- **DV图像**：`.jpg`, `.bmp`, `.png` 格式
- **时间戳**：从文件名提取微秒级时间戳

### 8.2 预期目录结构
```
session_folder/
├── dv_original/
│   ├── 1234567890123456.jpg
│   ├── 1234567890123457_cropped.jpg
│   └── ...
└── dvs_raw/
    └── data.raw
```

## 9. 错误处理与容错机制

### 9.1 文件缺失处理
- 自动跳过无效会话
- 降级到单模态回放
- 详细的日志输出

### 9.2 时间同步容错
- 预扫描失败时使用默认参数
- 时间戳解析错误时跳过该帧
- 超时保护机制

### 9.3 内存管理
- 智能指针管理OpenCV Mat对象
- 及时释放大图像数据
- 避免内存泄漏

这个完整的回放流程确保了高质量的多模态数据回放，支持实时检测集成，并提供了稳定的批量处理能力。
****
