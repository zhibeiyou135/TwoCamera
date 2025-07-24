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



# 检测功能完整流程详解

## 1. 检测功能架构概览

检测功能采用双摄像头架构，支持DV（普通摄像头）和DVS（事件摄像头）的独立检测，并通过结果管理器进行最终判决。

### 1.1 核心组件
- **DetectModule_DV**：DV摄像头检测模块
- **DetectModule_DVS**：DVS摄像头检测模块  
- **TorchEngine_DV/DVS**：PyTorch推理引擎
- **DualCameraResultManager**：双摄像头结果管理器

## 2. 检测功能启动流程

### 2.1 用户界面触发
````cpp path=mainwindow.cpp mode=EXCERPT
// 检测功能控件初始化
enableDetectionCheckBox = new QCheckBox(tr("启用检测功能"));
enableDetectionCheckBox->setChecked(false);  // 默认禁用
connect(enableDetectionCheckBox, &QCheckBox::toggled, this, &MainWindow::onDetectionToggled);

// 检测功能默认禁用
detectionEnabled = false;
enableDetect = false;
````

### 2.2 检测功能启用流程
````cpp path=mainwindow.cpp mode=EXCERPT
void MainWindow::onDetectionToggled(bool enabled) {
    detectionEnabled = enabled;
    enableDetect = enabled;  // 全局变量，供检测模块使用
    
    if (enabled) {
        // 启动检测功能
        if (!initializeDetectionModules()) {
            // 初始化失败，回退状态
            enableDetectionCheckBox->setChecked(false);
            detectionEnabled = false;
            enableDetect = false;
            return;
        }
        
        // 启动推理线程
        if (dvDetector && dvDetector->isModelLoaded()) {
            dvDetector->startInference();
        }
        if (dvsDetector && dvsDetector->isModelLoaded()) {
            dvsDetector->startInference();
        }
        
        // 切换到检测显示模式
        currentDisplayMode = DisplayMode::DETECTION_VIEW;
        
    } else {
        // 停止检测功能
        if (dvDetector) {
            dvDetector->stopInference();
        }
        if (dvsDetector) {
            dvsDetector->stopInference();
        }
        
        // 切换回相机显示模式
        currentDisplayMode = DisplayMode::CAMERA_VIEW;
    }
}
````

## 3. 检测模块初始化

### 3.1 模型加载流程
````cpp path=mainwindow.cpp mode=EXCERPT
bool MainWindow::initializeDetectionModules() {
    // 获取单例实例
    dvDetector = DetectModule_DV::getInstance();
    dvsDetector = DetectModule_DVS::getInstance();
    resultManager = DualCameraResultManager::getInstance();
    
    // 从配置文件加载模型路径
    QString modelPathDV = detectionConfig["models"]["dv_model_path"].toString();
    QString modelPathDVS = detectionConfig["models"]["dvs_model_path"].toString();
    
    // 加载模型
    bool dvModelLoaded = dvDetector->setModelPath(modelPathDV);
    bool dvsModelLoaded = dvsDetector->setModelPath(modelPathDVS);
    
    if (!dvModelLoaded || !dvsModelLoaded) {
        return false;
    }
    
    // 连接检测结果信号
    connect(dvDetector, &DetectModule_DV::newDetectResultImg, 
            this, &MainWindow::updateDVDetectionView);
    connect(dvDetector, &DetectModule_DV::newResult, 
            this, &MainWindow::onDVDetectionResult);
            
    connect(dvsDetector, &DetectModule_DVS::newDetectResultImg, 
            this, &MainWindow::updateDVSDetectionView);
    connect(dvsDetector, &DetectModule_DVS::newResult, 
            this, &MainWindow::onDVSDetectionResult);
    
    return true;
}
````

## 4. 图像采集与检测入队

### 4.1 DV图像处理流程
````cpp path=mainwindow.cpp mode=EXCERPT
// DV相机图像采集信号连接
connect(CameraCapture::getInstance(), &CameraCapture::captureImage, 
        this, &MainWindow::processDVImageForDetection);

// 独立的DV图像检测处理方法
void MainWindow::processDVImageForDetection(cv::Mat img) {
    if (!detectionEnabled || !dvDetector) {
        return;
    }

    // 检查图像有效性
    if (img.empty() || !img.data || img.rows <= 0 || img.cols <= 0) {
        return;
    }

    try {
        // 验证图像格式
        if (img.depth() != CV_8U || img.channels() != 3) {
            return;
        }

        // 创建检测图像副本
        cv::Mat detectImg;
        img.copyTo(detectImg);

        // 使用shared_ptr管理内存，确保线程安全
        std::shared_ptr<cv::Mat> imgPtr = std::make_shared<cv::Mat>(detectImg);
        dvDetector->enqueue(imgPtr);

    } catch (const std::exception& e) {
        qDebug() << "DV检测图像处理异常:" << e.what();
    }
}
````

### 4.2 DVS图像处理流程
````cpp path=mainwindow.cpp mode=EXCERPT
// DVS数据源信号连接
connect(DVSDataSource::getInstance(), &DVSDataSource::newImage, 
        this, &MainWindow::updateDVSView);

void MainWindow::updateDVSView(QImage img) {
    // UI显示逻辑（单一职责）
    if (currentDisplayMode == DisplayMode::CAMERA_VIEW) {
        ui->dvsLabel->setPixmap(QPixmap::fromImage(img.scaled(ui->dvsLabel->size(), 
                                Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    }
    
    // 检测处理分离到独立方法
    if (detectionEnabled) {
        QMetaObject::invokeMethod(this, "processDVSImageForDetection", 
                                 Qt::QueuedConnection, Q_ARG(QImage, img));
    }
}

// 独立的DVS图像检测处理方法
void MainWindow::processDVSImageForDetection(QImage img) {
    if (!detectionEnabled || !dvsDetector) {
        return;
    }

    // 使用统一的图像格式转换方法
    std::shared_ptr<cv::Mat> imgPtr = convertQImageToBGRMat(img);

    if (imgPtr && !imgPtr->empty()) {
        dvsDetector->enqueue(imgPtr);
    }
}
````

## 5. 检测模块内部处理

### 5.1 DV检测模块架构
````cpp path=infer/detect/DetectModule_DV.cpp mode=EXCERPT
class DVDetectWorker : public QThread {
public:
    DVDetectWorker(DetectModule_DV* parent) : parent(parent), shouldStop(false) {}
    
    void stopWorker() {
        shouldStop = true;
        wait();  // 等待线程结束
    }

protected:
    void run() override {
        auto engine = parent->getEngine();
        if (!engine) {
            return;
        }
        
        // 主处理循环
        while (!shouldStop) {
            try {
                // 非阻塞获取图像
                std::shared_ptr<cv::Mat> img;
                if (!parent->imgQueue.tryPop(img, 500)) {
                    continue;  // 500ms超时，继续检查shouldStop
                }
                
                if (shouldStop) break;
                
                // 图像有效性检查
                if (!img || img->empty() || img->data == nullptr) {
                    continue;
                }
                
                // 检查检测功能是否仍然启用
                extern bool enableDetect;
                if (!enableDetect) {
                    continue;
                }
                
                // 安全地克隆图像
                cv::Mat tensor;
                if (img->isContinuous()) {
                    tensor = img->clone();
                } else {
                    img->copyTo(tensor);
                }
                
                QString labels = "DV识别结果:";
                cv::Mat* result = nullptr;
                
                // 线程安全的推理
                {
                    std::lock_guard<std::mutex> lock(global_inference_mutex);
                    try {
                        result = engine->infer(&tensor, labels);
                    } catch (const std::exception& e) {
                        labels = "DV识别结果:推理失败";
                        result = &tensor;
                    }
                }
                
                // 发送结果
                if (result && !result->empty()) {
                    cv::Mat display_img;
                    if (result->channels() == 3) {
                        result->copyTo(display_img);
                    } else if (result->channels() == 1) {
                        cv::cvtColor(*result, display_img, cv::COLOR_GRAY2BGR);
                    }
                    
                    QImage qimg(display_img.data, display_img.cols, display_img.rows, 
                               display_img.step, QImage::Format_RGB888);
                    
                    if (!qimg.isNull()) {
                        emit parent->newDetectResultImg(qimg.copy());
                    }
                }
                
                emit parent->newResult(labels);
                
            } catch (const std::exception& e) {
                qDebug() << "DV检测: 处理循环异常:" << e.what();
                msleep(100);  // 短暂延时避免快速重试
            }
        }
    }

private:
    DetectModule_DV* parent;
    std::atomic<bool> shouldStop;
};
````

### 5.2 推理引擎核心逻辑
````cpp path=infer/detect/TorchEngine_DV.cpp mode=EXCERPT
cv::Mat* TorchEngine_DV::infer(cv::Mat* img, QString& labels) {
    try {
        // 1. 图像预处理
        cv::Mat resized_img;
        cv::resize(*img, resized_img, cv::Size(input_width, input_height));
        
        // 2. 归一化处理
        resized_img.convertTo(resized_img, CV_32F, 1.0 / 255.0);
        
        // 3. 转换为Tensor
        torch::Tensor tensor = torch::from_blob(resized_img.data, 
            {1, input_height, input_width, 3}, torch::kFloat);
        tensor = tensor.permute({0, 3, 1, 2});  // NHWC -> NCHW
        
        // 4. 模型推理
        std::vector<torch::jit::IValue> inputs;
        inputs.push_back(tensor.to(device));
        
        at::Tensor output = module.forward(inputs).toTensor();
        
        // 5. 后处理
        auto detections = postProcess(output);
        
        // 6. 绘制检测结果
        cv::Mat* result_img = new cv::Mat(img->clone());
        drawDetections(*result_img, detections, labels);
        
        return result_img;
        
    } catch (const std::exception& e) {
        labels = "DV识别结果:推理异常";
        return new cv::Mat(img->clone());
    }
}

std::vector<Detection> TorchEngine_DV::postProcess(const torch::Tensor& output) {
    std::vector<Detection> detections;
    
    // 1. 置信度筛选
    auto conf_mask = output.select(2, 4) > confidence_threshold;
    auto filtered_output = output.index({conf_mask});
    
    if (filtered_output.size(0) == 0) {
        return detections;
    }
    
    // 2. 坐标转换 (center_x, center_y, w, h) -> (x1, y1, x2, y2)
    auto boxes = filtered_output.slice(1, 0, 4);
    auto scores = filtered_output.slice(1, 4, 5);
    auto class_probs = filtered_output.slice(1, 5);
    
    // 3. 获取最高置信度类别
    auto max_scores = std::get<0>(torch::max(class_probs, 1));
    auto class_ids = std::get<1>(torch::max(class_probs, 1));
    
    // 4. NMS处理
    auto keep_indices = torch::ops::torchvision::nms(boxes, max_scores, iou_threshold);
    
    // 5. 构建最终检测结果
    for (int i = 0; i < keep_indices.size(0); i++) {
        int idx = keep_indices[i].item<int>();
        
        Detection det;
        det.bbox = extractBoundingBox(boxes[idx]);
        det.confidence = max_scores[idx].item<float>();
        det.class_id = class_ids[idx].item<int>();
        det.class_name = getClassName(det.class_id);
        
        detections.push_back(det);
    }
    
    return detections;
}
````

## 6. 检测结果处理与显示

### 6.1 检测结果接收
````cpp path=mainwindow.cpp mode=EXCERPT
void MainWindow::onDVDetectionResult(QString result) {
    if (detectionEnabled) {
        dvDetectionResultLabel->setText(tr("DV检测结果: ") + result);

        // 解析检测结果
        bool hasDetection = !result.contains("无");
        QString className = "";
        float confidence = 0.0f;

        if (hasDetection) {
            // 解析结果格式："DV识别结果: crab (0.85)"
            QStringList parts = result.split(" ");
            if (parts.size() >= 2) {
                className = parts[1];
                // 从括号中提取置信度
                QRegExp rx("\\((\\d+\\.\\d+)\\)");
                if (rx.indexIn(result) != -1) {
                    confidence = rx.cap(1).toFloat();
                }
            }

            // 检测时自动录制
            if (recordingWithDetection && hasDetection && !isRecording()) {
                auto path = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH_mm_ss") + "_dv_detect";
                emit this->startRecord(path);
            }
        }

        // 添加到结果管理器
        resultManager->addDVResult(result, hasDetection, className, confidence);
    }
}

void MainWindow::onDVSDetectionResult(QString result) {
    if (detectionEnabled) {
        dvsDetectionResultLabel->setText(tr("DVS检测结果: ") + result);

        // 类似的结果解析和处理逻辑
        bool hasDetection = !result.contains("无");
        QString className = "";
        float confidence = 0.0f;

        if (hasDetection) {
            // 解析逻辑同DV
            QStringList parts = result.split(" ");
            if (parts.size() >= 2) {
                className = parts[1];
                QRegExp rx("\\((\\d+\\.\\d+)\\)");
                if (rx.indexIn(result) != -1) {
                    confidence = rx.cap(1).toFloat();
                }
            }

            // 检测时自动录制
            if (recordingWithDetection && hasDetection && !isRecording()) {
                auto path = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH_mm_ss") + "_dvs_detect";
                emit this->startRecord(path);
            }
        }

        resultManager->addDVSResult(result, hasDetection, className, confidence);
    }
}
````

### 6.2 检测结果图像显示
````cpp path=mainwindow.cpp mode=EXCERPT
void MainWindow::updateDVDetectionView(QImage img) {
    if (detectionEnabled && currentDisplayMode == DisplayMode::DETECTION_VIEW) {
        // 在检测模式下显示带有检测框的图像
        ui->dvLabel->setPixmap(QPixmap::fromImage(img.scaled(ui->dvLabel->size(), 
                              Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    }
}

void MainWindow::updateDVSDetectionView(QImage img) {
    if (detectionEnabled && currentDisplayMode == DisplayMode::DETECTION_VIEW) {
        // 在检测模式下显示带有检测框的图像
        ui->dvsLabel->setPixmap(QPixmap::fromImage(img.scaled(ui->dvsLabel->size(), 
                               Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    }
}
````

## 7. 双摄像头结果融合

### 7.1 结果管理器架构
````cpp path=infer/detect/DualCameraResultManager.cpp mode=EXCERPT
class DualCameraResultManager : public QObject {
public:
    struct DetectionResult {
        QString result;
        bool hasDetection;
        QString className;
        float confidence;
        uint64_t timestamp;
    };

    void addDVResult(const QString& result, bool hasDetection, 
                     const QString& className, float confidence) {
        uint64_t timestamp = getCurrentTimestamp();
        
        DetectionResult dvResult;
        dvResult.result = result;
        dvResult.hasDetection = hasDetection;
        dvResult.className = className;
        dvResult.confidence = confidence;
        dvResult.timestamp = timestamp;
        
        dvResults.push_back(dvResult);
        
        // 查找时间窗口内的DVS结果进行融合
        processFinalDecision(timestamp);
    }

    void addDVSResult(const QString& result, bool hasDetection, 
                      const QString& className, float confidence) {
        uint64_t timestamp = getCurrentTimestamp();
        
        DetectionResult dvsResult;
        dvsResult.result = result;
        dvsResult.hasDetection = hasDetection;
        dvsResult.className = className;
        dvsResult.confidence = confidence;
        dvsResult.timestamp = timestamp;
        
        dvsResults.push_back(dvsResult);
        
        // 查找时间窗口内的DV结果进行融合
        processFinalDecision(timestamp);
    }

private:
    void processFinalDecision(uint64_t currentTimestamp) {
        const uint64_t timeWindow = 100;  // 100ms时间窗口
        
        // 查找时间窗口内的匹配结果
        auto dvMatch = findMatchingResult(dvResults, currentTimestamp, timeWindow);
        auto dvsMatch = findMatchingResult(dvsResults, currentTimestamp, timeWindow);
        
        if (dvMatch && dvsMatch) {
            // 双摄像头都有结果，进行融合判决
            QString finalDecision = makeFinalDecision(*dvMatch, *dvsMatch);
            
            // 保存到文件
            saveResultToFile(finalDecision, currentTimestamp);
            
            // 发出最终判决信号
            emit finalDecisionReady(finalDecision);
        }
    }

    QString makeFinalDecision(const DetectionResult& dvResult, 
                             const DetectionResult& dvsResult) {
        if (!dvResult.hasDetection && !dvsResult.hasDetection) {
            return "无检测结果";
        }
        
        if (dvResult.hasDetection && dvsResult.hasDetection) {
            if (dvResult.className == dvsResult.className) {
                // 一致检测：取平均置信度
                float avgConfidence = (dvResult.confidence + dvsResult.confidence) / 2.0f;
                return QString("一致检测: %1 (平均置信度: %2)")
                       .arg(dvResult.className).arg(avgConfidence, 0, 'f', 2);
            } else {
                // 冲突检测：选择置信度更高的
                if (dvResult.confidence > dvsResult.confidence) {
                    return QString("冲突检测(DV优先): %1 (置信度: %2) vs %3 (置信度: %4)")
                           .arg(dvResult.className).arg(dvResult.confidence, 0, 'f', 2)
                           .arg(dvsResult.className).arg(dvsResult.confidence, 0, 'f', 2);
                } else {
                    return QString("冲突检测(DVS优先): %1 (置信度: %2) vs %3 (置信度: %4)")
                           .arg(dvsResult.className).arg(dvsResult.confidence, 0, 'f', 2)
                           .arg(dvResult.className).arg(dvResult.confidence, 0, 'f', 2);
                }
            }
        }
        
        if (dvResult.hasDetection) {
            return QString("DV单独检测: %1 (置信度: %2)")
                   .arg(dvResult.className).arg(dvResult.confidence, 0, 'f', 2);
        }
        
        if (dvsResult.hasDetection) {
            return QString("DVS单独检测: %1 (置信度: %2)")
                   .arg(dvsResult.className).arg(dvsResult.confidence, 0, 'f', 2);
        }
        
        return "无检测结果";
    }
};
````

## 8. 回放模式下的检测集成

### 8.1 回放检测流程
````cpp path=mainwindow.cpp mode=EXCERPT
void MainWindow::onPlaybackImagePair(QImage dv, QImage dvs) {
    // 临时保存当前显示模式
    DisplayMode originalMode = currentDisplayMode;
    currentDisplayMode = DisplayMode::CAMERA_VIEW;

    // 处理DV图像
    if (!dv.isNull()) {
        // 统一的显示管理
        QMetaObject::invokeMethod(this, "updateCameraDisplay",
                                 Qt::QueuedConnection,
                                 Q_ARG(QImage, dv),
                                 Q_ARG(QString, "DV"));

        // 如果检测功能启用，使用统一的检测处理架构
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

        if (detectionEnabled) {
            QMetaObject::invokeMethod(this, "processDVSImageForDetection",
                                     Qt::QueuedConnection, Q_ARG(QImage, dvs));
        }
    }

    // 恢复原始显示模式
    currentDisplayMode = originalMode;
}
````

## 9. 配置管理

### 9.1 检测配置文件
````json path=detection_config.json mode=EXCERPT
{
  "detection": {
    "enabled": false,
    "time_window_ms": 100,
    "output_file": "detection_results.txt",
    "models": {
      "dv_model_path": "models/yolov5_dv.pt",
      "dvs_model_path": "models/yolov5_dvs.pt"
    },
    "parameters": {
      "dv": {
        "confidence_threshold": 0.45,
        "iou_threshold": 0.2,
        "input_width": 640,
        "input_height": 384,
        "num_classes": 10
      },
      "dvs": {
        "confidence_threshold": 0.45,
        "iou_threshold": 0.2,
        "input_width": 640,
        "input_height": 384,
        "num_classes": 10
      }
    }
  }
}
````

## 10. 支持的检测类别

系统支持10个预定义类别：
1. **starfish** (海星)
2. **seahorse** (海马)  
3. **crab** (螃蟹)
4. **turtle** (海龟)
5. **whale** (鲸鱼)
6. **car1** (汽车1)
7. **car2** (汽车2)
8. **car3** (汽车3)
9. **tank** (坦克)
10. **helicopter** (直升机)

## 11. 线程安全与性能优化

### 11.1 线程模型
- **主线程**：UI更新、信号槽处理
- **DV检测线程**：DV图像推理处理
- **DVS检测线程**：DVS图像推理处理
- **结果管理线程**：结果融合和文件保存

### 11.2 同步机制
- **全局推理互斥锁**：`global_inference_mutex`
- **阻塞队列**：`BlockingQueue<std::shared_ptr<cv::Mat>>`
- **原子变量**：`std::atomic<bool> enableDetect`

### 11.3 内存管理
- 使用`std::shared_ptr`管理OpenCV Mat对象
- 及时释放推理结果内存
- 避免图像数据的不必要拷贝

## 12. 错误处理与容错

### 12.1 模型加载失败处理
- 自动回退到非检测模式
- 详细的错误日志输出
- 用户友好的错误提示

### 12.2 推理异常处理
- 捕获所有推理异常
- 返回原始图像作为备选
- 继续处理后续帧

### 12.3 线程异常处理
- 工作线程异常自动恢复
- 短暂延时避免快速重试
- 保持系统稳定运行

这个完整的检测流程确保了高质量的双摄像头目标检测，支持实时和回放模式，并提供了稳定的结果融合机制。
****
