#ifndef DETECTION_SESSION_MANAGER_H
#define DETECTION_SESSION_MANAGER_H

#include <QString>
#include <QDateTime>
#include <QDir>
#include <QMutex>
#include <QDebug>
#include <atomic>
#include <memory>
#include <QHash>
#include <QtConcurrent/QtConcurrent>

/**
 * @brief 检测会话管理器 - 优化检测结果文件夹创建机制
 * 
 * 功能特性：
 * - 避免频繁创建文件夹，实现检测会话期间的文件夹复用
 * - 支持配置文件控制的文件夹创建策略
 * - 提供检测会话生命周期管理
 * - 与现有录制系统保持兼容
 */
class DetectionSessionManager {
public:
    static DetectionSessionManager* getInstance();
    
    // 配置管理
    struct DetectionSessionConfig {
        bool saveDetectionImages = true;        // 是否保存检测结果图片
        QString detectionImagePath = "detection_results";  // 检测结果基础路径
        bool createSessionFolders = true;       // 是否为每个检测会话创建独立文件夹
        
        void loadFromJson(const QJsonObject& detectionObj);
    };
    
    // 加载配置
    void loadConfiguration();
    
    // 会话管理
    void startDetectionSession();
    void endDetectionSession();
    void resetDetectionSession();
    bool isSessionActive() const;
    
    // 路径生成
    QString getDetectionResultPath(const QString& cameraType);
    QString getCurrentSessionPath() const;
    QString getSourceFolderName() const;

    // 会话验证
    bool isCurrentSessionValid() const;
    
    // 配置访问
    const DetectionSessionConfig& getConfig() const { return config; }
    bool shouldSaveDetectionImages() const { return config.saveDetectionImages; }
    
private:
    DetectionSessionManager();
    ~DetectionSessionManager() = default;
    
    // 禁用拷贝构造和赋值
    DetectionSessionManager(const DetectionSessionManager&) = delete;
    DetectionSessionManager& operator=(const DetectionSessionManager&) = delete;
    
    // 内部方法
    QString createSessionFolder();
    void ensureDetectionFolders(const QString& sessionPath);
    
    static DetectionSessionManager* instance;
    mutable QMutex sessionMutex;

    DetectionSessionConfig config;
    QString currentSessionPath;
    QDateTime sessionStartTime;
    std::atomic<bool> sessionActive{false};
    std::atomic<bool> configLoaded{false};

    // 路径缓存，避免重复计算和目录创建
    mutable QMutex pathCacheMutex;
    QHash<QString, QString> pathCache;

    // 源文件夹名称缓存
    mutable QString cachedSourceFolderName;
    mutable qint64 sourceFolderCacheTime;
};

#endif // DETECTION_SESSION_MANAGER_H
