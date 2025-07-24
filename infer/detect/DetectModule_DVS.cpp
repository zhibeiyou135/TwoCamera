#include "./DetectModule_DVS.h"
#include "./TorchEngine_DVS.h"
#include <QImage>
#include <QThread>
#include <QDebug>
#include <thread>
#include <vector>
#include <pthread.h>
#include <mutex>
#include <memory>

using namespace cv;

// 使用外部的全局推理锁
extern std::mutex global_inference_mutex;

// DVS检测工作线程类 - 简化版，只负责图像处理
class DVSDetectWorker : public QThread {
    Q_OBJECT
    
private:
    DetectModule_DVS* parent;
    volatile bool shouldStop;
    
public:
    DVSDetectWorker(DetectModule_DVS* p) : parent(p), shouldStop(false) {}
    
    void stopWorker() {
        shouldStop = true;
        wait(); // 等待线程结束
    }
    
protected:
    void run() override {
        // 设置线程名称
        pthread_setname_np(pthread_self(), "DVS_Detect");
        
        qDebug() << "DVS检测工作线程启动 - 线程ID:" << QThread::currentThreadId();
        
        // 检查模型是否已加载
        if (!parent->isModelLoaded()) {
            qDebug() << "DVS检测: 模型未加载，工作线程退出";
            emit parent->newResult("DVS识别结果:模型未加载");
            return;
        }
        
        TorchEngine_DVS* engine = parent->getEngine();
        if (!engine) {
            qDebug() << "DVS检测: 引擎指针为空，工作线程退出";
            emit parent->newResult("DVS识别结果:引擎未初始化");
            return;
        }
        
        qDebug() << "DVS检测引擎准备完成，开始处理队列";
        
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
                    qDebug() << "DVS检测: 跳过无效图像";
                    continue;
                }
                
                // 验证图像数据
                if (img->cols <= 0 || img->rows <= 0 || img->channels() <= 0) {
                    qDebug() << "DVS检测: 图像尺寸无效";
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
                        qDebug() << "DVS检测: 图像克隆失败";
                        continue;
                    }
                    
                } catch (const std::exception& e) {
                    qDebug() << "DVS检测: 图像处理异常:" << e.what();
                    continue;
                }
                
                QString labels = "DVS识别结果:";
                cv::Mat* result = nullptr;
                
                // 线程安全的推理
                {
                    std::lock_guard<std::mutex> lock(global_inference_mutex);
                    try {
                        result = engine->infer(&tensor, labels);
                    } catch (const std::exception& e) {
                        qDebug() << "DVS检测: 推理异常:" << e.what();
                        labels = "DVS识别结果:推理失败";
                        result = &tensor;
                    } catch (...) {
                        qDebug() << "DVS检测: 推理未知异常";
                        labels = "DVS识别结果:推理异常";
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
                            // 使用移动语义减少拷贝开销
                            QImage resultImg = qimg.copy();
                            emit parent->newDetectResultImg(std::move(resultImg));
                        }
                        
                    } catch (const std::exception& e) {
                        qDebug() << "DVS检测: 结果处理异常:" << e.what();
                    }
                }
                
                emit parent->newResult(labels);
                
            } catch (const std::exception& e) {
                qDebug() << "DVS检测: 处理循环异常:" << e.what();
                // 短暂延时避免快速重试
                msleep(100);
            } catch (...) {
                qDebug() << "DVS检测: 处理循环未知异常";
                msleep(100);
            }
        }
        
        qDebug() << "DVS检测工作线程结束";
    }
};

// DetectModule_DVS实现
DetectModule_DVS::DetectModule_DVS() 
    : worker(nullptr), modelLoaded(false), inferenceRunning(false) {
    qDebug() << "DetectModule_DVS 构造函数";
}

DetectModule_DVS::~DetectModule_DVS() {
    qDebug() << "DetectModule_DVS 析构函数";
    stopInference();
}

bool DetectModule_DVS::setModelPath(const QString &path) {
    qDebug() << "DetectModule_DVS: 设置模型路径并加载模型:" << path;
    
    // 如果推理正在运行，先停止
    if (inferenceRunning) {
        qDebug() << "DetectModule_DVS: 停止当前推理以重新加载模型";
        stopInference();
    }
    
    modelPath = path;
    modelLoaded = false;
    engine.reset(); // 重置现有引擎
    
    try {
        // 立即加载模型
        qDebug() << "DetectModule_DVS: 开始加载DVS模型...";
        engine = std::make_unique<TorchEngine_DVS>(path);
        
        if (engine) {
            modelLoaded = true;
            qDebug() << "DetectModule_DVS: DVS模型加载成功";
            emit modelLoadResult(true, "DVS模型加载成功");
            return true;
        } else {
            qDebug() << "DetectModule_DVS: DVS模型引擎创建失败";
            emit modelLoadResult(false, "DVS模型引擎创建失败");
            return false;
        }
        
    } catch (const std::exception& e) {
        qDebug() << "DetectModule_DVS: DVS模型加载异常:" << e.what();
        emit modelLoadResult(false, QString("DVS模型加载失败: %1").arg(e.what()));
        return false;
    } catch (...) {
        qDebug() << "DetectModule_DVS: DVS模型加载未知异常";
        emit modelLoadResult(false, "DVS模型加载失败: 未知异常");
        return false;
    }
}

void DetectModule_DVS::startInference() {
    qDebug() << "DetectModule_DVS: 启动推理线程";
    
    if (!modelLoaded) {
        qDebug() << "DetectModule_DVS: 模型未加载，无法启动推理";
        emit newResult("DVS识别结果:模型未加载");
        return;
    }
    
    if (inferenceRunning) {
        qDebug() << "DetectModule_DVS: 推理已在运行";
        return;
    }
    
    // 创建并启动工作线程
    worker = new DVSDetectWorker(this);
    worker->start();
    inferenceRunning = true;
    
    qDebug() << "DetectModule_DVS: 推理线程启动完成";
}

void DetectModule_DVS::stopInference() {
    qDebug() << "DetectModule_DVS: 停止推理线程";
    
    if (worker) {
        worker->stopWorker();
        delete worker;
        worker = nullptr;
    }
    
    inferenceRunning = false;
    qDebug() << "DetectModule_DVS: 推理线程已停止";
}

void DetectModule_DVS::enqueue(std::shared_ptr<cv::Mat> img) {
    if (img && !img->empty()) {
        this->imgQueue.push(img);
    } else {
        qDebug() << "DVS检测: 尝试入队无效图像";
    }
}

#include "DetectModule_DVS.moc" 