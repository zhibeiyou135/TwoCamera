#pragma once
#include "BlockingQueue.h"
#include "EventStruct.h"
#include "DVSBiasConfig.h"
#include <QByteArray>
#include <QImage>
#include <QObject>
#include <QDebug>
#include <array>
#include <boost/lockfree/spsc_queue.hpp>
#include <cstdint>
#include <fstream>
#include <memory>
#include <opencv2/opencv.hpp>
#include <vector>
// #include <metavision/sdk/core/utils/event_loop.h>
#include <metavision/sdk/core/algorithms/periodic_frame_generation_algorithm.h>
#include <metavision/sdk/driver/camera.h>
#include <metavision/sdk/base/events/event_cd.h>
#include <metavision/sdk/ui/utils/window.h>
#include <metavision/sdk/ui/utils/event_loop.h>
#include <metavision/hal/facilities/i_ll_biases.h>
#include <thread> 
#include <queue> 
#include <mutex> 
class EventAnalyzer {
public:
    // class variables to store global information
    int callback_counter               = 0; // this will track the number of callbacks
    int global_counter                 = 0; // this will track how many events we processed
    Metavision::timestamp global_max_t = 0; // this will track the highest timestamp we processed

    // this function will be associated to the camera callback
    // it is used to compute statistics on the received events
    void analyze_events(const Metavision::EventCD *begin, const Metavision::EventCD *end) {
        // time analysis
        // Note: events are ordered by timestamp in the callback, so the first event will have the lowest timestamp and
        // the last event will have the highest timestamp
        Metavision::timestamp min_t = begin->t;     // get the timestamp of the first event of this callback
        Metavision::timestamp max_t = (end - 1)->t; // get the timestamp of the last event of this callback
        global_max_t = max_t; // events are ordered by timestamp, so the current last event has the highest timestamp

        // counting analysis
        int counter = 0;
        for (const Metavision::EventCD *ev = begin; ev != end; ++ev) {
            ++counter; // increasing local counter
        }
        global_counter += counter; // increase global counter

        // Uncomment next line to display the buffer report in the terminal
        // WARNING : logging in the terminal can drastically decrease the performances of your application, especially
        // on embedded platforms with low computational power
//        std::cout << "Cb n°" << callback_counter << ": " << counter << " events from t=" << min_t << " to t="
//                  << max_t << " us." << std::endl;

        // increment callbacks counter
        callback_counter++;
    }
};

class DVSDataSource : public QObject {
  Q_OBJECT
private:
  static const int DVS_IMG_WIDTH = 1280, DVS_IMG_HEIGHT = 720;
  struct Event {
    int x, y, g;
    uint64_t t;
  };

  // 初始化相机，但不启动
  void initCamera();
  
  // 后处理函数，不包含启动相机的代码
  void postProcess();

  boost::lockfree::spsc_queue<QByteArray, boost::lockfree::capacity<500>>
      dataQueue;
  int allowAddDataFlag = 1;
  bool enableFrame = true;
  std::atomic<bool> enableDenoise{false};
  cv::Mat *imgBuffer;
  std::atomic<int> beilv{1};
  std::atomic<int> overLapCount{1};
  typedef std::array<std::shared_ptr<cv::Mat>, 2> WrapImage;
  //保存等待发射的图像数组，第一个元素是binary图像，第二个元素是累积gray图像，第三个元素是gray图像
  boost::lockfree::spsc_queue<WrapImage, boost::lockfree::capacity<5000>>
      waitForEmitImage;
  const int IMAGE_INTERVAL = 5000;
  std::atomic<bool> syncImageFlag{false};
  std::atomic<bool> recordFlag{false};
  boost::lockfree::spsc_queue<QByteArray, boost::lockfree::capacity<500>>
      dataSaveQueue;
  
  // 当前录制的RAW文件路径
  std::string rawFilePath;
  
  int imageEventCount = 0;
  uint64_t rowCount = 0;
  BlockingQueue<std::shared_ptr<cv::Mat>> postProcessQueue;

  //是否使能保存图片或RAW选项，由widgets中的SaveOptionsDialog控制
  std::atomic<bool> enableSaveRaw{true};
  std::atomic<bool> enableSaveImg{true};
  std::string saveFolderPath;
  int fpnData[1280 * 720];
  Metavision::Camera cam;       // create the camera
  EventAnalyzer event_analyzer; // create the event analyzer
  
  // 自动启动标志，默认为false
  std::atomic<bool> autoStart{false};
  
  // 相机是否已初始化和运行的标志
  std::atomic<bool> cameraInitialized{false};
  std::atomic<bool> cameraRunning{false};
  
  // 相机操作的互斥锁
  std::mutex cameraMutex;
  
  void showImg(WrapImage &imgs);

  // 以下成员已废弃，现在使用FileSaveManager处理文件保存
  // std::thread saveThread[15];
  // std::queue<cv::Mat> imageQueue; // 队列，用于存储需要保存的图片
  // std::mutex queueMutex; // 互斥量，用于保护队列的线程安全
  // std::condition_variable queueCondVar; // 条件变量，用于通知保存线程有新图片
  // void saveImageThread();
  
  // 静态成员，用于保存帧生成器和后处理线程
  static std::shared_ptr<Metavision::PeriodicFrameGenerationAlgorithm> frameGenerator;
  static std::thread postProcessThread;
  static bool postProcessThreadStarted;

  // DVS帧率设置
  std::atomic<double> currentFPS{20.0};

public:
  DVSDataSource();
  ~DVSDataSource();

  void addData(QByteArray data);

  static bool isNoise(uint64_t *data, const int x, const int y);

  static DVSDataSource *getInstance() {
    static DVSDataSource instance;
    return &instance;
  }
  void loadFpn(const QString &fpn);
  
  // 设置是否自动启动相机
  void setAutoStart(bool enable) {
    autoStart.store(enable);
    qDebug() << "DVSDataSource autoStart set to" << enable;
  }
  
  // 获取自动启动状态
  bool getAutoStart() const {
    return autoStart.load();
  }

public slots:
  // 明确的相机启动方法
  void startCamera();
  
  // 明确的相机停止方法
  void stopCamera();

  void setDenoise(bool enable);

  void setSpeed(int b);

  void setOverlapCount(int o);

  void syncImage();

  void startRecord(const QString &path);

  void stopRecord();

  void setEnableSaveRaw(bool f) { enableSaveRaw.store(f); }

  void setEnableSaveImg(bool f) { enableSaveImg.store(f); }
  bool getEnableSaveImg() { return enableSaveImg.load(); }
  bool getEnableSaveRaw() { return enableSaveRaw.load(); }

  // 设置保存文件夹路径
  void setSaveFolderPath(const QString &path) { saveFolderPath = path.toStdString(); }
  
  // 获取保存文件夹路径
  QString getSaveFolderPath() const { return QString::fromStdString(saveFolderPath); }
  
  // 设置DVS帧率
  void setFPS(double fps);
  
  // 获取当前DVS帧率
  double getFPS() const;

  /**
   * @brief 应用偏置设置到DVS相机
   */
  bool applyBiasSettings();
  
  /**
   * @brief 从配置文件加载偏置设置
   */
  void loadBiasSettings();
  
  /**
   * @brief 保存当前偏置设置到文件
   */
  bool saveBiasToFile(const QString& filePath);

signals:

  void newImage(QImage);
  void newBinaryImage(QImage);
  void newGrayImage(QImage);
  void newAccGrayImage(QImage);
  void newConstructGrayImage(QImage);
  
  // 相机启动和停止的信号
  void cameraStarted();
  void cameraStopped();
};



