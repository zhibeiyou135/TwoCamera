//
// Created for TwoCamera_v6detect project
// 文件保存管理器 - 解决录制过程中UI卡死问题
//

#pragma once

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QString>
#include <QImage>
#include <opencv2/opencv.hpp>
#include <atomic>
#include <memory>

// 前向声明
class DetectionSessionManager;

// 文件保存任务结构体
struct FileSaveTask {
    enum TaskType {
        SAVE_DV_ORIGINAL,
        SAVE_DV_CROPPED,
        SAVE_DVS_IMAGE,
        SAVE_DETECTION_RESULT,
        SAVE_DV_DETECTION_IMAGE,    // DV检测结果图像
        SAVE_DVS_DETECTION_IMAGE    // DVS检测结果图像
    };

    TaskType type;
    cv::Mat image;           // OpenCV图像数据
    QImage qimage;          // Qt图像数据
    QString filePath;       // 保存路径
    QString content;        // 文本内容（用于检测结果）
    uint64_t timestamp;     // 时间戳
    int priority;           // 任务优先级 (0=最高, 9=最低)

    FileSaveTask() : type(SAVE_DV_ORIGINAL), timestamp(0), priority(5) {}
};

// 文件保存工作线程
class FileSaveWorker : public QObject {
    Q_OBJECT
    
public:
    FileSaveWorker();
    ~FileSaveWorker();
    
    void addTask(const FileSaveTask &task);
    void stop();
    
public slots:
    void processQueue();
    
signals:
    void taskCompleted(const QString &filePath, bool success);
    void queueSizeChanged(int size);
    void saveProgress(int completed, int total);
    
private:
    QQueue<FileSaveTask> taskQueue;
    QMutex queueMutex;
    QWaitCondition queueCondition;
    std::atomic<bool> running;
    std::atomic<int> completedTasks;
    std::atomic<int> totalTasks;
    
    bool saveOpenCVImage(const cv::Mat &image, const QString &filePath);
    bool saveQImage(const QImage &image, const QString &filePath);
    bool saveTextFile(const QString &content, const QString &filePath);
};

// 文件保存管理器主类
class FileSaveManager : public QObject {
    Q_OBJECT
    
public:
    static FileSaveManager* getInstance();
    
    // 启动和停止文件保存服务
    void startService();
    void stopService();
    
    // 添加保存任务的便捷方法
    void saveDVOriginal(const cv::Mat &image, const QString &filePath, uint64_t timestamp = 0);
    void saveDVCropped(const cv::Mat &image, const QString &filePath, uint64_t timestamp = 0);
    void saveDVSImage(const cv::Mat &image, const QString &filePath, uint64_t timestamp = 0);
    void saveDetectionResult(const QString &content, const QString &filePath, uint64_t timestamp = 0);

    // 检测结果图像保存方法 - 优化版本
    void saveDVDetectionImage(const QImage &image, const QString &filePath, uint64_t timestamp = 0, int priority = 3);
    void saveDVSDetectionImage(const QImage &image, const QString &filePath, uint64_t timestamp = 0, int priority = 3);

    // 智能限流的检测结果保存
    void saveDetectionImageWithThrottling(const QImage &image, const QString &cameraType, uint64_t timestamp = 0, const QString &originalFilename = QString());
    
    // 获取队列状态
    int getQueueSize() const;
    int getCompletedTasks() const;
    int getTotalTasks() const;

    // 清空队列
    void clearQueue();

    // 性能监控
    double getAverageSaveTime() const;
    int getFailedTaskCount() const;
    
signals:
    void saveCompleted(const QString &filePath, bool success);
    void queueStatusChanged(int queueSize, int completed, int total);
    void serviceStarted();
    void serviceStopped();
    
private slots:
    void onTaskCompleted(const QString &filePath, bool success);
    void onQueueSizeChanged(int size);
    void onSaveProgress(int completed, int total);
    
private:
    FileSaveManager();
    ~FileSaveManager();
    
    static FileSaveManager* instance;
    QThread* workerThread;
    FileSaveWorker* worker;
    std::atomic<bool> serviceRunning;
    
    // 统计信息
    std::atomic<int> currentQueueSize;
    std::atomic<int> completedTaskCount;
    std::atomic<int> totalTaskCount;

    // 限流控制
    std::atomic<uint64_t> lastDVDetectionSave;
    std::atomic<uint64_t> lastDVSDetectionSave;
    static constexpr uint64_t DETECTION_SAVE_INTERVAL_MS = 2000; // 2秒间隔
};
