#pragma once

#include <QJsonObject>
#include <QDebug>
#include <map>
#include <string>

/**
 * @brief DVS偏置配置结构体
 * 
 * 基于Prophesee文档的偏置参数配置
 * 支持IMX636、Gen4.1、GenX320等传感器
 */
struct DVSBiasConfig {
    bool enabled = false;           // 是否启用偏置设置
    int bias_diff = 0;             // 差分偏置（推荐不修改）
    int bias_diff_on = 0;          // ON事件对比度阈值偏置
    int bias_diff_off = 0;         // OFF事件对比度阈值偏置
    int bias_fo = 0;               // 低通滤波器偏置
    int bias_hpf = 0;              // 高通滤波器偏置
    int bias_refr = 0;             // 不应期偏置
    
    /**
     * @brief 从JSON对象加载偏置配置
     */
    void loadFromJson(const QJsonObject& biasObj) {
        if (biasObj.contains("enabled") && biasObj["enabled"].isBool()) {
            enabled = biasObj["enabled"].toBool();
        }
        
        if (biasObj.contains("bias_diff") && biasObj["bias_diff"].isDouble()) {
            bias_diff = biasObj["bias_diff"].toInt();
        }
        
        if (biasObj.contains("bias_diff_on") && biasObj["bias_diff_on"].isDouble()) {
            bias_diff_on = biasObj["bias_diff_on"].toInt();
        }
        
        if (biasObj.contains("bias_diff_off") && biasObj["bias_diff_off"].isDouble()) {
            bias_diff_off = biasObj["bias_diff_off"].toInt();
        }
        
        if (biasObj.contains("bias_fo") && biasObj["bias_fo"].isDouble()) {
            bias_fo = biasObj["bias_fo"].toInt();
        }
        
        if (biasObj.contains("bias_hpf") && biasObj["bias_hpf"].isDouble()) {
            bias_hpf = biasObj["bias_hpf"].toInt();
        }
        
        if (biasObj.contains("bias_refr") && biasObj["bias_refr"].isDouble()) {
            bias_refr = biasObj["bias_refr"].toInt();
        }
        
        qDebug() << "DVS偏置配置加载完成:";
        qDebug() << "  enabled:" << enabled;
        qDebug() << "  bias_diff:" << bias_diff;
        qDebug() << "  bias_diff_on:" << bias_diff_on;
        qDebug() << "  bias_diff_off:" << bias_diff_off;
        qDebug() << "  bias_fo:" << bias_fo;
        qDebug() << "  bias_hpf:" << bias_hpf;
        qDebug() << "  bias_refr:" << bias_refr;
    }
    
    /**
     * @brief 获取偏置映射表（用于Metavision SDK）
     */
    std::map<std::string, int> getBiasMap() const {
        std::map<std::string, int> biasMap;
        biasMap["bias_diff"] = bias_diff;
        biasMap["bias_diff_on"] = bias_diff_on;
        biasMap["bias_diff_off"] = bias_diff_off;
        biasMap["bias_fo"] = bias_fo;
        biasMap["bias_hpf"] = bias_hpf;
        biasMap["bias_refr"] = bias_refr;
        return biasMap;
    }
    
    /**
     * @brief 验证偏置值是否在有效范围内（IMX636传感器）
     */
    bool validateBiases() const {
        // IMX636传感器的偏置范围
        const std::map<std::string, std::pair<int, int>> ranges = {
            {"bias_diff", {-25, 23}},
            {"bias_diff_on", {-85, 140}},
            {"bias_diff_off", {-35, 190}},
            {"bias_fo", {-35, 55}},
            {"bias_hpf", {0, 120}},
            {"bias_refr", {-20, 235}}
        };
        
        auto checkRange = [&](const std::string& name, int value) -> bool {
            auto it = ranges.find(name);
            if (it != ranges.end()) {
                if (value < it->second.first || value > it->second.second) {
                    qDebug() << "偏置值超出范围:" << name.c_str() << "=" << value 
                             << ", 有效范围: [" << it->second.first << "," << it->second.second << "]";
                    return false;
                }
            }
            return true;
        };
        
        return checkRange("bias_diff", bias_diff) &&
               checkRange("bias_diff_on", bias_diff_on) &&
               checkRange("bias_diff_off", bias_diff_off) &&
               checkRange("bias_fo", bias_fo) &&
               checkRange("bias_hpf", bias_hpf) &&
               checkRange("bias_refr", bias_refr);
    }
};

/**
 * @brief DVS偏置管理器单例类
 */
class DVSBiasManager {
public:
    static DVSBiasManager& getInstance() {
        static DVSBiasManager instance;
        return instance;
    }
    
    /**
     * @brief 设置偏置配置
     */
    void setBiasConfig(const DVSBiasConfig& config) {
        biasConfig = config;
    }
    
    /**
     * @brief 获取偏置配置
     */
    const DVSBiasConfig& getBiasConfig() const {
        return biasConfig;
    }
    
    /**
     * @brief 从JSON对象加载偏置配置
     */
    void loadFromJson(const QJsonObject& biasObj) {
        biasConfig.loadFromJson(biasObj);
    }
    
private:
    DVSBiasManager() = default;
    DVSBiasConfig biasConfig;
}; 