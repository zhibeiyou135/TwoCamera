#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QString>

class ConfigManager {
public:
    static ConfigManager& getInstance() {
        static ConfigManager instance;
        return instance;
    }
    
    // 设置配置文件路径
    void setConfigPath(const QString& path) {
        configPath = path;
    }
    
    // 获取配置文件路径
    QString getConfigPath() const {
        return configPath.isEmpty() ? "dv_dvs.json" : configPath;
    }
    
private:
    ConfigManager() = default;
    QString configPath;
};

#endif // CONFIGMANAGER_H 