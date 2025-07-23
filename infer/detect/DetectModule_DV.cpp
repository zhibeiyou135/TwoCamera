#include "./DetectModule_DV.h"
#include "./TorchEngine_DV.h"
#include <QImage>
#include <QThread>
#include <QDebug>
#include <thread>
#include <vector>
#include <pthread.h>
#include <mutex>
#include <memory>
using namespace cv;

// 全局推理锁，确保推理操作的线程安全
std::mutex global_inference_mutex;

// 检测工作线程类 - 简化版，只负责图像处理
class DVDetectWorker : public QThread {
    Q_OBJECT
    
private:
    DetectModule_DV* parent;
    volatile bool shouldStop;
    
public:
    DVDetectWorker(DetectModule_DV* p) : parent(p), shouldStop(false) {}
    
    void stopWorker() {
        shouldStop = true;
        wait(); // 等待线程结束
    }
    
protected:
    void run() override {
        // 设置线程名称
        pthread_setname_np(pthread_self(), "DV_Detect");
        
        qDebug() << "DV检测工作线程启动 - 线程ID:" << QThread::currentThreadId();
        
        // 检查模型是否已加载
        if (!parent->isModelLoaded()) {
            qDebug() << "DV检测: 模型未加载，工作线程退出";
            emit parent->newResult("DV识别结果:模型未加载");
            return;
        }
        
        TorchEngine_DV* engine = parent->getEngine();
        if (!engine) {
            qDebug() << "DV检测: 引擎指针为空，工作线程退出";
            emit parent->newResult("DV识别结果:引擎未初始化");
            return;
        }
        
        qDebug() << "DV检测引擎准备完成，开始处理队列";
        
        // 主处理循环
        while (!shouldStop) {
            try {
                // 使用非阻塞的tryPop避免无限等待
                std::shared_ptr<cv::Mat> img;
                if (!parent->imgQueue.tryPop(img, 500)) {
                    // 如果500ms内没有新图像，继续循环检查shouldStop
                    continue;
                }
                
                if (shouldStop) break;
                
                // 检查图像有效性
                if (!img || img->empty() || img->data == nullptr) {
                    qDebug() << "DV检测: 跳过无效图像";
                    continue;
                }
                
                // 验证图像数据
                if (img->cols <= 0 || img->rows <= 0 || img->channels() <= 0) {
                    qDebug() << "DV检测: 图像尺寸无效";
                    continue;
                }
                
                // 检查检测功能是否仍然启用
                extern bool enableDetect;
                if (!enableDetect) {
                    // 如果检测功能被禁用，跳过处理但继续循环
                    continue;
                }
                
                // 安全地克隆图像
                cv::Mat tensor;
                try {
                    if (img->isContinuous()) {
                        tensor = img->clone();
                    } else {
                        img->copyTo(tensor);
                    }
                    
                    if (tensor.empty() || !tensor.data) {
                        qDebug() << "DV检测: 图像克隆失败";
                        continue;
                    }
                    
                } catch (const std::exception& e) {
                    qDebug() << "DV检测: 图像处理异常:" << e.what();
                    continue;
                }
                
                QString labels = "DV识别结果:";
                cv::Mat* result = nullptr;
                
                // 线程安全的推理
                {
                    std::lock_guard<std::mutex> lock(global_inference_mutex);
                    try {
                        result = engine->infer(&tensor, labels);
                    } catch (const std::exception& e) {
                        qDebug() << "DV检测: 推理异常:" << e.what();
                        labels = "DV识别结果:推理失败";
                        result = &tensor;
                    } catch (...) {
                        qDebug() << "DV检测: 推理未知异常";
                        labels = "DV识别结果:推理异常";
                        result = &tensor;
                    }
                }
                
                // 发送结果
                if (result && !result->empty()) {
                    try {
                        cv::Mat display_img;
                        if (result->channels() == 3) {
                            result->copyTo(display_img);
                        } else if (result->channels() == 1) {
                            cv::cvtColor(*result, display_img, cv::COLOR_GRAY2BGR);
                        } else {
                            result->copyTo(display_img);
                        }
                        
                        QImage qimg(display_img.data, display_img.cols, display_img.rows, 
                                   display_img.step, QImage::Format_RGB888);
                        
                        if (!qimg.isNull()) {
                            emit parent->newDetectResultImg(qimg.copy());
                        }
                        
                    } catch (const std::exception& e) {
                        qDebug() << "DV检测: 结果处理异常:" << e.what();
                    }
                }
                
                emit parent->newResult(labels);
                
            } catch (const std::exception& e) {
                qDebug() << "DV检测: 处理循环异常:" << e.what();
                // 短暂延时避免快速重试
                msleep(100);
            } catch (...) {
                qDebug() << "DV检测: 处理循环未知异常";
                msleep(100);
            }
        }
        
        qDebug() << "DV检测工作线程结束";
    }
};

// DetectModule_DV实现
DetectModule_DV::DetectModule_DV() 
    : worker(nullptr), modelLoaded(false), inferenceRunning(false) {
    qDebug() << "DetectModule_DV 构造函数";
}

DetectModule_DV::~DetectModule_DV() {
    qDebug() << "DetectModule_DV 析构函数";
    stopInference();
}

bool DetectModule_DV::setModelPath(const QString &path) {
    qDebug() << "DetectModule_DV: 设置模型路径并加载模型:" << path;
    
    // 如果推理正在运行，先停止
    if (inferenceRunning) {
        qDebug() << "DetectModule_DV: 停止当前推理以重新加载模型";
        stopInference();
    }
    
    modelPath = path;
    modelLoaded = false;
    engine.reset(); // 重置现有引擎
    
    try {
        // 立即加载模型
        qDebug() << "DetectModule_DV: 开始加载DV模型...";
        engine = std::make_unique<TorchEngine_DV>(path);
        
        if (engine) {
            modelLoaded = true;
            qDebug() << "DetectModule_DV: DV模型加载成功";
            emit modelLoadResult(true, "DV模型加载成功");
            return true;
        } else {
            qDebug() << "DetectModule_DV: DV模型引擎创建失败";
            emit modelLoadResult(false, "DV模型引擎创建失败");
            return false;
        }
        
    } catch (const std::exception& e) {
        qDebug() << "DetectModule_DV: DV模型加载异常:" << e.what();
        emit modelLoadResult(false, QString("DV模型加载失败: %1").arg(e.what()));
        return false;
    } catch (...) {
        qDebug() << "DetectModule_DV: DV模型加载未知异常";
        emit modelLoadResult(false, "DV模型加载失败: 未知异常");
        return false;
    }
}

void DetectModule_DV::startInference() {
    qDebug() << "DetectModule_DV: 启动推理线程";
    
    if (!modelLoaded) {
        qDebug() << "DetectModule_DV: 模型未加载，无法启动推理";
        emit newResult("DV识别结果:模型未加载");
        return;
    }
    
    if (inferenceRunning) {
        qDebug() << "DetectModule_DV: 推理已在运行";
        return;
    }
    
    // 创建并启动工作线程
    worker = new DVDetectWorker(this);
    worker->start();
    inferenceRunning = true;
    
    qDebug() << "DetectModule_DV: 推理线程启动完成";
}

void DetectModule_DV::stopInference() {
    qDebug() << "DetectModule_DV: 停止推理线程";
    
    if (worker) {
        worker->stopWorker();
        delete worker;
        worker = nullptr;
    }
    
    inferenceRunning = false;
    qDebug() << "DetectModule_DV: 推理线程已停止";
}

void DetectModule_DV::enqueue(std::shared_ptr<cv::Mat> img) {
    if (img && !img->empty()) {
        this->imgQueue.push(img);
    } else {
        qDebug() << "DV检测: 尝试入队无效图像";
    }
}

#include "DetectModule_DV.moc"
