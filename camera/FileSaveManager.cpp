//
// Created for TwoCamera_v6detect project
// 文件保存管理器实现 - 解决录制过程中UI卡死问题
//

#include "FileSaveManager.h"
#include "RecordingConfig.h"
#include "DetectionSessionManager.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QDateTime>
#include <QMutexLocker>
#include <QDateTime>
#include <chrono>
#include <opencv2/opencv.hpp>

// FileSaveWorker 实现
FileSaveWorker::FileSaveWorker() 
    : running(false), completedTasks(0), totalTasks(0) {
}

FileSaveWorker::~FileSaveWorker() {
    stop();
}

void FileSaveWorker::addTask(const FileSaveTask &task) {
    QMutexLocker locker(&queueMutex);

    // 简单的优先级插入：高优先级任务插入到队列前部
    if (task.priority <= 2 && !taskQueue.isEmpty()) {
        // 为高优先级任务预留位置，插入到队列前部
        QQueue<FileSaveTask> tempQueue;
        tempQueue.enqueue(task);
        while (!taskQueue.isEmpty()) {
            tempQueue.enqueue(taskQueue.dequeue());
        }
        taskQueue = tempQueue;
    } else {
        taskQueue.enqueue(task);
    }

    totalTasks.fetch_add(1);
    queueCondition.wakeOne();

    emit queueSizeChanged(taskQueue.size());
}

void FileSaveWorker::stop() {
    running.store(false);
    queueCondition.wakeAll();
}

void FileSaveWorker::processQueue() {
    running.store(true);
    qDebug() << "文件保存工作线程启动";
    
    while (running.load()) {
        FileSaveTask task;
        bool hasTask = false;
        
        // 获取任务
        {
            QMutexLocker locker(&queueMutex);
            if (taskQueue.isEmpty()) {
                queueCondition.wait(&queueMutex, 100); // 等待100ms
                continue;
            }
            
            task = taskQueue.dequeue();
            hasTask = true;
            emit queueSizeChanged(taskQueue.size());
        }
        
        if (!hasTask) continue;
        
        // 处理任务
        bool success = false;
        QString filePath = task.filePath;
        
        try {
            // 确保目录存在
            QFileInfo fileInfo(filePath);
            QDir().mkpath(fileInfo.absolutePath());
            
            switch (task.type) {
                case FileSaveTask::SAVE_DV_ORIGINAL:
                case FileSaveTask::SAVE_DV_CROPPED:
                case FileSaveTask::SAVE_DVS_IMAGE:
                    success = saveOpenCVImage(task.image, filePath);
                    break;

                case FileSaveTask::SAVE_DV_DETECTION_IMAGE:
                case FileSaveTask::SAVE_DVS_DETECTION_IMAGE:
                    success = saveQImage(task.qimage, filePath);
                    break;

                case FileSaveTask::SAVE_DETECTION_RESULT:
                    success = saveTextFile(task.content, filePath);
                    break;
                    
                default:
                    qDebug() << "未知的文件保存任务类型:" << task.type;
                    break;
            }
            
        } catch (const std::exception &e) {
            qDebug() << "文件保存异常:" << e.what() << "文件:" << filePath;
            success = false;
        }
        
        // 更新统计信息
        int completed = completedTasks.fetch_add(1) + 1;
        emit taskCompleted(filePath, success);
        emit saveProgress(completed, totalTasks.load());
        
        if (!success) {
            qDebug() << "文件保存失败:" << filePath;
        }
    }
    
    qDebug() << "文件保存工作线程停止";
}

bool FileSaveWorker::saveOpenCVImage(const cv::Mat &image, const QString &filePath) {
    if (image.empty()) {
        qDebug() << "图像为空，无法保存:" << filePath;
        return false;
    }
    
    try {
        bool result = cv::imwrite(filePath.toStdString(), image);
        if (!result) {
            qDebug() << "图像保存失败:" << filePath;
        }
        return result;
    } catch (const cv::Exception &e) {
        qDebug() << "OpenCV保存图像异常:" << e.what() << "文件:" << filePath;
        return false;
    }
}

bool FileSaveWorker::saveQImage(const QImage &image, const QString &filePath) {
    if (image.isNull()) {
        qDebug() << "QImage为空，无法保存:" << filePath;
        return false;
    }
    
    bool result = image.save(filePath);
    if (!result) {
        qDebug() << "QImage保存失败:" << filePath;
    }
    return result;
}

bool FileSaveWorker::saveTextFile(const QString &content, const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件进行写入:" << filePath;
        return false;
    }
    
    QTextStream out(&file);
    out << content;
    file.close();

    return true;
}

// FileSaveManager 实现
FileSaveManager* FileSaveManager::instance = nullptr;

FileSaveManager::FileSaveManager()
    : workerThread(nullptr), worker(nullptr), serviceRunning(false),
      currentQueueSize(0), completedTaskCount(0), totalTaskCount(0),
      lastDVDetectionSave(0), lastDVSDetectionSave(0) {
}

FileSaveManager::~FileSaveManager() {
    stopService();
}

FileSaveManager* FileSaveManager::getInstance() {
    if (!instance) {
        instance = new FileSaveManager();
    }
    return instance;
}

void FileSaveManager::startService() {
    if (serviceRunning.load()) {
        qDebug() << "文件保存服务已经在运行";
        return;
    }
    
    qDebug() << "启动文件保存服务";
    
    // 创建工作线程和工作对象
    workerThread = new QThread(this);
    worker = new FileSaveWorker();
    worker->moveToThread(workerThread);
    
    // 连接信号
    connect(workerThread, &QThread::started, worker, &FileSaveWorker::processQueue);
    connect(worker, &FileSaveWorker::taskCompleted, this, &FileSaveManager::onTaskCompleted);
    connect(worker, &FileSaveWorker::queueSizeChanged, this, &FileSaveManager::onQueueSizeChanged);
    connect(worker, &FileSaveWorker::saveProgress, this, &FileSaveManager::onSaveProgress);
    
    // 启动线程
    workerThread->start();
    serviceRunning.store(true);
    
    emit serviceStarted();
    qDebug() << "文件保存服务启动完成";
}

void FileSaveManager::stopService() {
    if (!serviceRunning.load()) {
        return;
    }
    
    qDebug() << "停止文件保存服务";
    serviceRunning.store(false);
    
    if (worker) {
        worker->stop();
    }
    
    if (workerThread) {
        workerThread->quit();
        if (!workerThread->wait(5000)) {
            qDebug() << "工作线程停止超时，强制终止";
            workerThread->terminate();
            workerThread->wait(1000);
        }
        
        delete worker;
        worker = nullptr;
        
        workerThread->deleteLater();
        workerThread = nullptr;
    }
    
    emit serviceStopped();
    qDebug() << "文件保存服务停止完成";
}

void FileSaveManager::saveDVOriginal(const cv::Mat &image, const QString &filePath, uint64_t timestamp) {
    if (!serviceRunning.load()) {
        qDebug() << "文件保存服务未启动，无法保存DV原始图像";
        return;
    }
    
    FileSaveTask task;
    task.type = FileSaveTask::SAVE_DV_ORIGINAL;
    task.image = image.clone(); // 深拷贝避免数据竞争
    task.filePath = filePath;
    task.timestamp = timestamp;
    
    worker->addTask(task);
}

void FileSaveManager::saveDVCropped(const cv::Mat &image, const QString &filePath, uint64_t timestamp) {
    if (!serviceRunning.load()) {
        qDebug() << "文件保存服务未启动，无法保存DV裁剪图像";
        return;
    }
    
    FileSaveTask task;
    task.type = FileSaveTask::SAVE_DV_CROPPED;
    task.image = image.clone(); // 深拷贝避免数据竞争
    task.filePath = filePath;
    task.timestamp = timestamp;
    
    worker->addTask(task);
}

void FileSaveManager::saveDVSImage(const cv::Mat &image, const QString &filePath, uint64_t timestamp) {
    if (!serviceRunning.load()) {
        qDebug() << "文件保存服务未启动，无法保存DVS图像";
        return;
    }
    
    FileSaveTask task;
    task.type = FileSaveTask::SAVE_DVS_IMAGE;
    task.image = image.clone(); // 深拷贝避免数据竞争
    task.filePath = filePath;
    task.timestamp = timestamp;
    
    worker->addTask(task);
}

void FileSaveManager::saveDetectionResult(const QString &content, const QString &filePath, uint64_t timestamp) {
    if (!serviceRunning.load()) {
        qDebug() << "文件保存服务未启动，无法保存检测结果";
        return;
    }

    FileSaveTask task;
    task.type = FileSaveTask::SAVE_DETECTION_RESULT;
    task.content = content;
    task.filePath = filePath;
    task.timestamp = timestamp;
    task.priority = 4; // 中等优先级

    worker->addTask(task);
}

void FileSaveManager::saveDVDetectionImage(const QImage &image, const QString &filePath, uint64_t timestamp, int priority) {
    if (!serviceRunning.load()) {
        qDebug() << "文件保存服务未启动，无法保存DV检测结果图像";
        return;
    }

    if (image.isNull()) {
        qDebug() << "DV检测结果图像为空，跳过保存";
        return;
    }

    FileSaveTask task;
    task.type = FileSaveTask::SAVE_DV_DETECTION_IMAGE;
    task.qimage = image.copy(); // 深拷贝确保线程安全
    task.filePath = filePath;
    task.timestamp = timestamp;
    task.priority = priority;

    worker->addTask(task);
}

void FileSaveManager::saveDVSDetectionImage(const QImage &image, const QString &filePath, uint64_t timestamp, int priority) {
    if (!serviceRunning.load()) {
        qDebug() << "文件保存服务未启动，无法保存DVS检测结果图像";
        return;
    }

    if (image.isNull()) {
        qDebug() << "DVS检测结果图像为空，跳过保存";
        return;
    }

    FileSaveTask task;
    task.type = FileSaveTask::SAVE_DVS_DETECTION_IMAGE;
    task.qimage = image.copy(); // 深拷贝确保线程安全
    task.filePath = filePath;
    task.timestamp = timestamp;
    task.priority = priority;

    worker->addTask(task);
}

void FileSaveManager::saveDetectionImageWithThrottling(const QImage &image, const QString &cameraType, uint64_t timestamp) {
    if (!serviceRunning.load() || image.isNull()) {
        return;
    }

    // 获取检测会话管理器
    auto sessionManager = DetectionSessionManager::getInstance();

    // 检查是否应该保存检测图片
    if (!sessionManager->shouldSaveDetectionImages()) {
        return;
    }

    // 获取当前时间戳（毫秒）
    uint64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    // 检查限流
    std::atomic<uint64_t>* lastSaveTime = nullptr;
    if (cameraType == "DV") {
        lastSaveTime = &lastDVDetectionSave;
    } else if (cameraType == "DVS") {
        lastSaveTime = &lastDVSDetectionSave;
    } else {
        qDebug() << "未知的相机类型:" << cameraType;
        return;
    }

    uint64_t lastTime = lastSaveTime->load();
    if (currentTime - lastTime < FileSaveManager::DETECTION_SAVE_INTERVAL_MS) {
        // 限流：跳过此次保存
        return;
    }

    // 更新最后保存时间
    lastSaveTime->store(currentTime);

    // 使用检测会话管理器获取检测结果路径
    QString detectResultPath = sessionManager->getDetectionResultPath(cameraType);

    if (detectResultPath.isEmpty()) {
        qDebug() << "检测结果路径为空，跳过保存";
        return;
    }

    // 生成文件名 - 格式: {源文件夹名称}_{相机类型}_detection_{时间戳}_{微秒时间戳}.png
    QString sourceFolderName = sessionManager->getSourceFolderName();
    QString timestampStr = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss-zzz");
    QString filename = QString("%1/%2_%3_detection_%4_%5.png")
                      .arg(detectResultPath)
                      .arg(sourceFolderName)
                      .arg(cameraType.toLower())
                      .arg(timestampStr)
                      .arg(timestamp);

    // 异步保存
    if (cameraType == "DV") {
        saveDVDetectionImage(image, filename, timestamp, 3);
    } else {
        saveDVSDetectionImage(image, filename, timestamp, 3);
    }
}

int FileSaveManager::getQueueSize() const {
    return currentQueueSize.load();
}

int FileSaveManager::getCompletedTasks() const {
    return completedTaskCount.load();
}

int FileSaveManager::getTotalTasks() const {
    return totalTaskCount.load();
}

void FileSaveManager::clearQueue() {
    if (worker) {
        // 这里可以添加清空队列的逻辑
        qDebug() << "清空文件保存队列";
    }
}

double FileSaveManager::getAverageSaveTime() const {
    // 简单实现，返回估算的平均保存时间
    return 50.0; // 毫秒
}

int FileSaveManager::getFailedTaskCount() const {
    // 简单实现，可以后续扩展
    return 0;
}

void FileSaveManager::onTaskCompleted(const QString &filePath, bool success) {
    completedTaskCount.fetch_add(1);
    emit saveCompleted(filePath, success);
}

void FileSaveManager::onQueueSizeChanged(int size) {
    currentQueueSize.store(size);
    emit queueStatusChanged(size, completedTaskCount.load(), totalTaskCount.load());
}

void FileSaveManager::onSaveProgress(int completed, int total) {
    emit queueStatusChanged(currentQueueSize.load(), completed, total);
}
