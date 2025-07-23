#pragma once
#include <QObject>
#include <QString>
#include <QDateTime>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>
#include <QFile>
#include <deque>
#include <memory>

struct DetectionResult {
    QString cameraType;      // "DV" or "DVS"
    QString detectionInfo;   // 检测结果信息
    QDateTime timestamp;     // 检测时间戳
    bool hasDetection;       // 是否有有效检测
    QString className;       // 检测到的类别名称
    float confidence;        // 置信度
};

class DualCameraResultManager : public QObject {
    Q_OBJECT
    
private:
    DualCameraResultManager(QObject *parent = nullptr) : QObject(parent) {}
    
    // 存储检测结果的队列
    std::deque<DetectionResult> dvResults;
    std::deque<DetectionResult> dvsResults;
    
    // 时间窗口设置（毫秒）
    int timeWindowMs = 500;  // 500ms内的结果认为是同一时刻
    
    // 结果队列最大长度
    const int maxQueueSize = 100;
    
    // 互斥锁保护数据
    QMutex resultMutex;
    
    // 定时器，用于定期处理结果
    QTimer *processTimer;
    
    // 输出文件
    QString outputFilePath;
    
    // 处理配对的结果
    void processMatchedResults();
    
    // 清理过期的结果
    void cleanupOldResults();
    
    // 写入最终判决到文件
    void writeFinalDecision(const QString &decision, const QDateTime &timestamp);
    
    // 根据DV和DVS的结果产生最终判决
    QString generateFinalDecision(const DetectionResult &dvResult, const DetectionResult &dvsResult);
    
public:
    static DualCameraResultManager* getInstance() {
        static DualCameraResultManager instance;
        return &instance;
    }
    
    // 添加析构函数
    ~DualCameraResultManager();
    
    // 初始化管理器
    void initialize(const QString &outputFile = "detection_results.txt");
    
    // 设置时间窗口
    void setTimeWindow(int milliseconds) { timeWindowMs = milliseconds; }
    
    // 添加DV检测结果
    void addDVResult(const QString &detectionInfo, bool hasDetection = true, 
                     const QString &className = "", float confidence = 0.0f);
    
    // 添加DVS检测结果
    void addDVSResult(const QString &detectionInfo, bool hasDetection = true, 
                      const QString &className = "", float confidence = 0.0f);

private slots:
    // 定期处理结果的槽函数
    void onProcessResults();

signals:
    // 产生最终判决时发出的信号
    void finalDecisionMade(QString decision, QDateTime timestamp);
}; 