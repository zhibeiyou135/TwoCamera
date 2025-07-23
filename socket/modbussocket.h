#ifndef MODBUSSOCKET_H
#define MODBUSSOCKET_H
#include <iostream>
#include <modbus/modbus.h>
#include <QObject>
#include <QDebug>
#include <chrono>
#include <thread>
#include <QtConcurrent>
#include "TransData.h"
#include <QTimer>
using namespace std;
class modbusSocket: public QObject
{
    Q_OBJECT
public:
    modbusSocket();
    modbusSocket(char* ip,int port);
    modbus_t *ctx;
    bool write_rotate(modbusSocket);
    int rc;
    uint8_t value;
    int xMaxData; 
    int xMinData;
    int xMidData;
    int zMAXData;
    int zMinData;
    int rMaxData;
    int rMinData;
    int yMAXData;
    int half_x;
    int rNow;
    bool open_flag;
    char* ip;
    int port;
    int rotate_position_address;
    int rotate_flag_address;
    int open_address;
    int x_address;
    int y_address;
    int z_address;
    int detect_finally_address;
    int detect_res_address;
    int debug_mode;
    int rotate_speed_address;
    int rotate_speed;
    int sleep_time ;
    int x_speed;
    int xNow;
    uint8_t x_flag[32];
    uint8_t y_flag[32];
    uint8_t z_flag[32];
    // uint8_t num[32];
    bool detect_flag;
    uint8_t detect_flag1[32];
    uint8_t detect_flag2[32];
    bool getImgFlag;
    time_t start;
    time_t end;
    QString time_path;
    
    // 添加循环旋转相关的成员变量
    bool isLoopRotating;
    QTimer* rotateTimer;
    bool isForwardRotation;
    
    // 添加自动录制配置参数
    int autoRecording_x1;
    int autoRecording_x2;  
    int autoRecording_x3;
    int autoRecording_x4;  // 添加第四个X轴位置
    int autoRecording_y;
    int autoRecording_z;   // 添加Z轴位置参数
    int autoRecording_totalDiscs;
    QString autoRecording_basePath;
    
    void run();
public slots:
    void open();
    void rotate();
    void revent();
    void q_open();
    void q_close();
    void x_mobile();
    void z_mobile();
    void z_mobile(int zData);  // 添加带参数的Z轴移动函数
    void detectRes();
    void detectFinally();
    void rotate_x_mobile();
    void revent_x_mobile();
    void click_x();
    void click_rotate();
    void debug();  
    void click_rotate_x();
    void click_rotate_x_45();
    void revent_x_step();
    void rotate_x_step();
    void y_micoreDV();
    void y_micore(int yData);
    void basic_rotate(int Rdata);
    void x_basic_move(int Xdata);
    void emit_path(QString path,QString time_path,QString root_path );
    void reset();
    void begin();
    void stop_emergency();
    
    // 添加循环旋转的槽函数
    void startLoopRotation();
    void stopLoopRotation();
    void executeRotationCycle();
    
    // 添加新的测试函数
    void testDetectFinally();
    void testDetectRes();
    
    // 添加测试写入0的函数
    void testDetectFinallyZero();
    void testDetectResZero();
    
    // 添加训练顺逆旋转功能（不移动Y轴）
    void startTrainingRotation();
    void stopTrainingRotation();
    void executeTrainingRotationCycle();
    
    // 添加清料、换料操作
    void clearMaterial();
    void changeMaterial();
    
    // 添加自动录制相关方法
    void performAutoRecording();
    void recordSingleDisc(int discNumber);
    void recordAtPosition(int xPosition, const QString& suffix);
    void loadAutoRecordingConfig();
    
    // 添加X1-X4位置的录制和移动功能
    void recordAtX1();    // X1位置录制
    void recordAtX2();    // X2位置录制
    void recordAtX3();    // X3位置录制
    void recordAtX4();    // X4位置录制
    void moveToX1();      // 移动到X1位置
    void moveToX2();      // 移动到X2位置
    void moveToX3();      // 移动到X3位置
    void moveToX4();      // 移动到X4位置
    
signals:
    void startRecord(QString);
    void stopRecord();
    void discRecordingComplete(int discNumber);
    void autoRecordingComplete();
};



#endif // MODBUSSOCKET_H
