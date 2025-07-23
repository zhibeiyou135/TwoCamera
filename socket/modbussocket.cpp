#include "modbussocket.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include "camera/ConfigManager.h"
#include "camera/RecordingConfig.h"

modbusSocket::modbusSocket()
{

}
modbusSocket::modbusSocket(char* ip,int port){
    this->ip = ip;
    this->port = port;
    this->ctx = modbus_new_tcp(ip, port);
    this->rotate_position_address = 41538;   //写入旋转角度的地址
    this->rotate_flag_address = 0;           //写入开始旋转的信号
    this->open_address = 24584;
    this->x_address = 80;
    this->y_address = 82;
    this->detect_finally_address = 1403;
    this->detect_res_address = 1404;
    this->xMaxData = 33700;
    this->xMinData = 13000;
    this->xMidData = 24000;
    this->zMinData = 0;
    this->zMAXData = 15000;
    this->rMaxData = 42000;
    this->yMAXData = 29200;
    this->rMinData = 0;
    this->rNow = 0;
    this->z_address = 84;
    this->detect_flag = true;
    this->getImgFlag = false;
    this->open_flag = false;
    this->debug_mode = 0;
    this->rotate_speed_address = 41540;
    this->rotate_speed = 2500;
    this->xNow = 24000;
    this->x_speed = 5500;
    this->half_x = 1500;

    // 初始化循环旋转相关变量
    this->isLoopRotating = false;
    this->isForwardRotation = true;
    this->rotateTimer = new QTimer(this);
    connect(this->rotateTimer, &QTimer::timeout, this, &modbusSocket::executeRotationCycle);

    // 初始化自动录制配置参数（默认值）
    this->autoRecording_x1 = 19000;
    this->autoRecording_x2 = 24000;
    this->autoRecording_x3 = 29000;
    this->autoRecording_x4 = 32000;  // 添加第四个X轴位置默认值
    this->autoRecording_y = 23000;
    this->autoRecording_z = 7500;    // 添加Z轴位置默认值
    this->autoRecording_totalDiscs = 25;
    this->autoRecording_basePath = "/home/pe/recordings/auto_batch";
    
    // 加载配置文件
    loadAutoRecordingConfig();
}
void modbusSocket::open(){
    qDebug() << "=== 开始连接Modbus服务器 ===";
    qDebug() << "IP地址:" << this->ip;
    qDebug() << "端口:" << this->port;
    
    int connectResult = modbus_connect(ctx);
    qDebug() << "连接结果:" << connectResult;
    
    if (connectResult == -1) {
        std::cerr << "无法连接到Modbus服务器: " << modbus_strerror(errno) << std::endl;
        qDebug() << "连接失败，错误信息:" << modbus_strerror(errno);
        modbus_free(ctx);
     } else {
        qDebug() << "Modbus服务器连接成功！";
     }
     this->open_flag = true;
     this->debug_mode = 0;
     qDebug() << "=== Modbus连接完成 ===";
}

void modbusSocket::basic_rotate(int Rdata){
    qDebug() << "=== 开始旋转操作 ===";
    qDebug() << "目标旋转位置:" << Rdata;
    qDebug() << "当前旋转位置:" << this->rNow;
    qDebug() << "旋转速度:" << this->rotate_speed;
    rc = modbus_write_register(this->ctx, this->rotate_speed_address, this->rotate_speed); 
    if(rc == -1){
        qDebug()<<"旋转速度址写入失败! 地址:" << this->rotate_speed_address<< "值:" << this->rotate_speed;
    }
    else{
        qDebug()<<"旋转速度址写入成功! 地址:" << this->rotate_speed_address<< "值:" << this->rotate_speed;
    }
    rc = modbus_write_register(this->ctx, this->rotate_position_address,Rdata);
    if(rc == -1){
        qDebug()<<"旋转位置地址写入失败! 地址:" << this->rotate_position_address << "值:" << Rdata;
    }
    else{
        qDebug()<<"旋转位置地址写入成功! 地址:" << this->rotate_position_address << "值:" << Rdata;
    }
    
    if (this->rNow < Rdata)
    {
        this->sleep_time = ceil((double)(Rdata - this->rNow)/this->rotate_speed);
        qDebug() << "正向旋转，预计耗时:" << this->sleep_time << "秒";
    }
    else{
        this->sleep_time = ceil((double)(this->rNow - Rdata)/this->rotate_speed);
        qDebug() << "反向旋转，预计耗时:" << this->sleep_time << "秒";
    }
    
    this->value = 1;
    rc = modbus_write_bit(this->ctx, this->rotate_flag_address, this->value);
    if(rc == -1){
        qDebug()<<"旋转启动标志写入失败! 地址:" << this->rotate_flag_address;
    }
    else{
        qDebug()<<"旋转启动标志写入成功! 地址:" << this->rotate_flag_address;
        qDebug()<<"开始等待旋转完成，等待时间:" << this->sleep_time << "秒";
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(this->sleep_time ));    
    
    this->rNow = Rdata;
    qDebug() << "旋转完成，当前位置更新为:" << this->rNow;
    qDebug() << "=== 旋转操作结束 ===";
}

void modbusSocket::x_basic_move(int Xdata)
{
    qDebug() << "=== 开始X轴移动操作 ===";
    qDebug() << "目标X轴位置:" << Xdata;
    qDebug() << "当前X轴位置:" << this->xNow;
    qDebug() << "X轴移动速度:" << this->x_speed;
    
    rc = modbus_write_register(this->ctx, this->x_address, Xdata);
    if(rc == -1){
        qDebug()<<"X轴位置写入失败! 地址:" << this->x_address << "值:" << Xdata;
    }
    else{
        qDebug()<<"X轴位置写入成功! 地址:" << this->x_address << "值:" << Xdata;
    }
    
    if (Xdata < this->xNow)
    {
        this->sleep_time = (this->xNow - Xdata)/this->x_speed ;
        qDebug() << "X轴负向移动，预计耗时:" << this->sleep_time << "秒";
    }
    else{
        this->sleep_time = (Xdata - this->xNow)/this->x_speed ;
        qDebug() << "X轴正向移动，预计耗时:" << this->sleep_time << "秒";
    }
    
    qDebug() << "开始等待X轴移动完成，等待时间:" << this->sleep_time << "秒";
    std::this_thread::sleep_for(std::chrono::seconds(this->sleep_time));
    this->xNow = Xdata;
    qDebug() << "X轴移动完成，当前位置更新为:" << this->xNow;
    qDebug() << "=== X轴移动操作结束 ===";
}
void modbusSocket::rotate(){
    qDebug() << "=== 执行正向旋转(rotate) ===";
    qDebug() << "目标位置:" << this->rMaxData;
    basic_rotate(this->rMaxData);
}

void modbusSocket::revent(){
    qDebug() << "=== 执行反向旋转(revent) ===";
    qDebug() << "目标位置:" << this->rMinData;
    basic_rotate(this->rMinData);
}

void modbusSocket::q_open(){
    qDebug() << "=== 执行松开操作(q_open) ===";
    qDebug() << "设置开关地址:" << this->open_address << "值: 0";
    this->value = 0;
    rc = modbus_write_bit(this->ctx, this->open_address, this->value);
    if(rc == -1){
        qDebug()<<"松开操作写入失败! 地址:" << this->open_address;
    } else {
        qDebug()<<"松开操作写入成功! 地址:" << this->open_address;
    }
    qDebug() << "=== 松开操作完成 ===";
}

void modbusSocket::q_close(){
    qDebug() << "=== 执行抓取操作(q_close) ===";
    qDebug() << "设置开关地址:" << this->open_address << "值: 1";
    this->value = 1;
    rc = modbus_write_bit(this->ctx, this->open_address, this->value);
    if(rc == -1){
        qDebug()<<"抓取操作写入失败! 地址:" << this->open_address;
    } else {
        qDebug()<<"抓取操作写入成功! 地址:" << this->open_address;
    }
    qDebug() << "=== 抓取操作完成 ===";
}
void modbusSocket::x_mobile(){
    qDebug() << "=== 执行X轴移动序列(x_mobile) ===";
    qDebug() << "移动序列: MAX(" << this->xMaxData << ") -> MIN(" << this->xMinData << ") -> MID(" << this->xMidData << ")";
    x_basic_move(this->xMaxData);
    x_basic_move(this->xMinData);
    x_basic_move(this->xMidData);
    qDebug() << "=== X轴移动序列完成 ===";
}
void modbusSocket::z_mobile(){
    qDebug() << "=== 执行Z轴移动序列(z_mobile) ===";
    qDebug() << "移动序列: MAX(" << this->zMAXData << ") -> MIN(" << this->zMinData << ") -> MAX(" << this->zMAXData << ")";
    qDebug() << "Z轴地址:" << this->z_address;
    
    // 第一步：移动到最大位置
    qDebug() << "第一步：Z轴移动到最大位置" << this->zMAXData;
    rc = modbus_write_register(this->ctx, this->z_address, this->zMAXData);
    if(rc == -1){
        qDebug()<<"Z轴移动到最大位置失败! 地址:" << this->z_address << "值:" << this->zMAXData;
    }
    else{
        qDebug()<<"Z轴移动到最大位置成功! 地址:" << this->z_address << "值:" << this->zMAXData;
    }
    qDebug() << "等待1秒...";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 第二步：移动到最小位置
    qDebug() << "第二步：Z轴移动到最小位置" << this->zMinData;
    rc = modbus_write_register(this->ctx, this->z_address, this->zMinData);
    if(rc == -1)
    {
        qDebug()<<"Z轴移动到最小位置失败! 地址:" << this->z_address << "值:" << this->zMinData;
    }
    else
    {
        qDebug()<<"Z轴移动到最小位置成功! 地址:" << this->z_address << "值:" << this->zMinData;
    }
    qDebug() << "等待1秒...";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 第三步：移动回最大位置
    qDebug() << "第三步：Z轴移动回最大位置" << this->zMAXData;
    rc = modbus_write_register(this->ctx, this->z_address, this->zMAXData);
    if(rc == -1)
    {
        qDebug()<<"Z轴移动回最大位置失败! 地址:" << this->z_address << "值:" << this->zMAXData;
    }
    else
    {
        qDebug()<<"Z轴移动回最大位置成功! 地址:" << this->z_address << "值:" << this->zMAXData;
    }
    qDebug() << "=== Z轴移动序列完成 ===";
    // if (rc == -1)modbus_connect(ctx);
}

void modbusSocket::z_mobile(int zData){
    qDebug() << "=== 执行Z轴移动到指定位置(z_mobile) ===";
    qDebug() << "目标Z轴位置:" << zData;
    qDebug() << "Z轴地址:" << this->z_address;
    
    // 移动到指定位置
    qDebug() << "Z轴移动到位置:" << zData;
    rc = modbus_write_register(this->ctx, this->z_address, zData);
    if(rc == -1){
        qDebug()<<"Z轴移动失败! 地址:" << this->z_address << "值:" << zData;
    }
    else{
        qDebug()<<"Z轴移动成功! 地址:" << this->z_address << "值:" << zData;
    }
    qDebug() << "等待1秒...";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    qDebug() << "=== Z轴移动到指定位置完成 ===";
}

void modbusSocket::y_micore(int yData){
    qDebug() << "=== 执行Y轴移动(y_micore) ===";
    qDebug() << "目标Y轴位置:" << yData;
    qDebug() << "Y轴地址:" << this->y_address;
    
    rc = modbus_write_register(this->ctx, this->y_address,yData);
    if(rc == -1){
        qDebug()<<"Y轴位置写入失败! 地址:" << this->y_address << "值:" << yData;
    }
    else{
        qDebug()<<"Y轴位置写入成功! 地址:" << this->y_address << "值:" << yData;
    }
    qDebug() << "Y轴移动等待1秒...";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    qDebug() << "=== Y轴移动完成 ===";
}
void modbusSocket::y_micoreDV(){
    // y_micore();
    rotate();
    revent();
}

void modbusSocket::revent_x_mobile(){
    
    this->value = 1;
    rc = modbus_write_register(this->ctx, this->rotate_position_address,this->rMinData);
    this->rNow = this->rMinData;
    if(rc == -1){
        qDebug()<<"rotate_position_address write fail!";
    }
    else{
        qDebug()<<"rotate_position_address write sucess!";
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));

    
    rc = modbus_write_bit(this->ctx, this->rotate_flag_address, this->value);
    if(rc == -1){
        qDebug()<<"rotate_flag_address write fail!";
    }
    else{
        qDebug()<<"rotate_flag_address write sucess!";
    }

        rc = modbus_write_register(this->ctx, this->x_address, this->xMaxData);
    if(rc == -1){
        qDebug()<<"write fail!";
    }
    else{
        qDebug()<<"write sucess!";
    }
     std::this_thread::sleep_for(std::chrono::seconds(1));
     rc = modbus_write_register(this->ctx, this->x_address, this->xMinData);
     if(rc == -1)
     {
         qDebug()<<"write fail!";
     }
     else
     {
         qDebug()<<"write sucess!";
     }
     std::this_thread::sleep_for(std::chrono::seconds(1));
     rc = modbus_write_register(this->ctx, this->x_address, this->xMidData);
     if(rc == -1)
     {
         qDebug()<<"write fail!";
     }
     else
     {
         qDebug()<<"write sucess!";
     }

}
void modbusSocket::rotate_x_mobile(){
    
    rc = modbus_write_register(this->ctx, this->rotate_position_address,this->rMaxData);
    this->rNow = this->rMaxData;
    if(rc == -1){
        qDebug()<<"rotate_position_address write fail!";
    }
    else{
        qDebug()<<"rotate_position_address write sucess!";
    }
    //电缸旋转方式
    this->value = 1;
    
    rc = modbus_write_bit(this->ctx, this->rotate_flag_address, this->value);
    if(rc == -1){
        qDebug()<<"rotate_flag_address write fail!";
    }
    else{
        qDebug()<<"rotate_flag_address write sucess!";
    }
        rc = modbus_write_register(this->ctx, this->x_address, this->xMaxData);
    if(rc == -1){
        qDebug()<<"write fail!";
    }
    else{
        qDebug()<<"write sucess!";
    }
     std::this_thread::sleep_for(std::chrono::seconds(1));
     rc = modbus_write_register(this->ctx, this->x_address, this->xMinData);
     if(rc == -1)
     {
         qDebug()<<"write fail!";
     }
     else
     {
         qDebug()<<"write sucess!";
     }
     std::this_thread::sleep_for(std::chrono::seconds(1));
     rc = modbus_write_register(this->ctx, this->x_address, this->xMidData);
     if(rc == -1)
     {
         qDebug()<<"write fail!";
     }
     else
     {
         qDebug()<<"write sucess!";
     }
     if (rc == -1)modbus_connect(ctx);
}
void modbusSocket::detectFinally(){
    this->value = 1;
    rc = modbus_write_bit(this->ctx, this->detect_finally_address, this->value);
    if(rc == -1){
        qDebug()<<"write fail!";
    }
}

void modbusSocket::rotate_x_step(){
    
    // 每次旋转45度，然后x轴运动
    basic_rotate(this->rNow + 4500);
}

void modbusSocket::revent_x_step(){
    basic_rotate(this->rNow - 4500);

}
void modbusSocket::detectRes(){
    this->value = 0;
    rc = modbus_write_bit(this->ctx, this->open_address, this->value);
    if(rc == -1){
        qDebug()<<"write fail!";
    }
}
void modbusSocket::click_x()
{
    this->debug_mode = 2;
}
void modbusSocket::click_rotate()
{
    this->debug_mode = 1;
}
void modbusSocket::click_rotate_x()
{
    this->debug_mode = 3;
}

void modbusSocket::click_rotate_x_45()
{
    this->debug_mode = 4;
}
void modbusSocket::debug(){
    
    while(true){
        if (this->debug_mode == 1)
        {
                rotate();
                revent();
        }
        if (this->debug_mode == 2)
        {
                // x_mobile();
        }
        if (this->debug_mode == 3)
        {
                rotate_x_mobile();
                std::this_thread::sleep_for(std::chrono::seconds(6));
                revent_x_mobile();
                std::this_thread::sleep_for(std::chrono::seconds(6));
        }
        if (this->debug_mode == 4)
        {
            rotate_x_step();
            x_mobile();
            if (this->rNow >= this->rMaxData)
            {
                this->debug_mode = 5;
            }
        }
        if (this->debug_mode == 5)
        {
            revent_x_step();

            x_mobile();
            
            if (this->rNow <= this->rMinData)
            {
                this->debug_mode = 4;
            }
            
        } 
        if (this->debug_mode == 0)
        {
            break;
        }
    }
}

void modbusSocket::emit_path(QString path,QString time_path,QString root_path )
{
    emit this->startRecord(root_path +"record/" +path+"/"+time_path);
}

void modbusSocket::stop_emergency()
{
    qDebug() << "=== 执行紧急停止(stop_emergency) ===";
    qDebug() << "紧急停止地址: 102, 值: 1";
    this->value =1;
    rc = modbus_write_bit(this->ctx, 102,this->value); 
    if(rc == -1){
        qDebug()<<"紧急停止写入失败!";
    } else {
        qDebug()<<"紧急停止写入成功!";
    }
    qDebug() << "=== 紧急停止操作完成 ===";
}
void modbusSocket::begin(){
    qDebug() << "=== 执行开始操作(begin) ===";
    qDebug() << "开始地址: 100, 值: 1";
    this->value = 1;
    rc = modbus_write_bit(this->ctx, 100, this->value);
    if(rc == -1){
        qDebug()<<"开始操作写入失败!";
    } else {
        qDebug()<<"开始操作写入成功!";
    }
    qDebug() << "=== 开始操作完成 ===";
}

void modbusSocket::reset(){
    qDebug() << "=== 执行复位操作(reset) ===";
    qDebug() << "复位地址: 104, 值: 1";
    this->value = 1;
    rc = modbus_write_bit(this->ctx, 104, this->value);
    if(rc == -1){
        qDebug()<<"复位操作写入失败!";
    } else {
        qDebug()<<"复位操作写入成功!";
    }
    qDebug() << "=== 复位操作完成 ===";
}



void modbusSocket::run(){
  QtConcurrent::run([this]() {
    int num= 0;
    int norm = 2;
    // QString root_path = "";
    QString root_path = "/media/pe/Camera_1/test_kasi";
    while(true){
        rc = modbus_read_bits(ctx,1500,1,x_flag);
        modbus_read_bits(ctx,1501,1,y_flag);
        modbus_read_bits(ctx,1502,1,z_flag);
        modbus_read_bits(ctx,1503,1,detect_flag1);
        modbus_read_bits(ctx,1504,1,detect_flag2);
        modbus_write_register(this->ctx, this->rotate_speed_address, this->rotate_speed); 
        if (this ->open_flag && rc == -1)
        {
            open();
            qDebug()<<"zidongchonglian:";
        }
        if (this ->debug_mode)
        {
            debug();
        }
        
        if(detect_flag1[0] == (uint8_t)1 || detect_flag2[0] == (uint8_t)1){
            num++;
            
            time_path = QString::number(num);
                            
            if (norm == 1)
            {
                QString path = "旋转";
                // emit this->startRecord(path+"/"+time_path);
                // emit_path(path,time_path,root_path);
                // rotate();
                // // emit this->stopRecord();
                // // std::this_thread::sleep_for(std::chrono::seconds(20));
                // // emit_path(path,time_path,root_path);
                // revent();
                // emit this->stopRecord();
                
            }
            else if (norm == 2)
            {
                QString path = "旋转";
                y_micore(this->yMAXData);
                x_basic_move(this->xMidData +2500);
                emit_path(path,time_path,root_path);
                // emit this->startRecord(path+"/"+time_path);
                rotate();
                emit this->stopRecord();
                std::this_thread::sleep_for(std::chrono::seconds(35));
                emit_path(path,time_path,root_path);
                revent();
                emit this->stopRecord();
            }
            else if (norm == 3)
            {
                QString path = "旋转";
                y_micore(23000);
                x_basic_move(this->xMidData +300);
                emit_path(path,time_path,root_path);
                // emit this->startRecord(path+"/"+time_path);
                rotate();
                x_basic_move(this->xMidData +1600);
                revent();
                emit this->stopRecord();
            }
            else if (norm == 4)
            {
                QString path = "旋转";
       
                emit_path(path,time_path,root_path);
                // emit this->startRecord(path+"/"+time_path);
                rotate();
                emit this->stopRecord();
                std::this_thread::sleep_for(std::chrono::seconds(5));
                emit_path(path,time_path,root_path);
                revent();
                
                emit this->stopRecord();
                std::this_thread::sleep_for(std::chrono::seconds(5));
                // QString path1 = "x轴移动";
                // // emit this->startRecord(path1+"/"+time_path);
                // emit_path(path1,time_path,root_path);
                
                // x_mobile();
                // emit this->stopRecord();


                // QString path3 = "45度平移";
                // emit this->startRecord(path3+"/"+time_path);
                
                // for (;this->rNow < this->rMaxData; )
                // {
                //     emit_path(path3,time_path,root_path);
                //     rotate_x_step();
                //     x_mobile();
                //     emit this->stopRecord();
                //     std::this_thread::sleep_for(std::chrono::seconds(4));
                // }
                // this->rNow = 0;
                
                // emit this->stopRecord();
            }


            if (this->debug_mode )
            {
                debug();
            }
            detectFinally();
            std::this_thread::sleep_for(std::chrono::seconds(1));
            //modbus_read_bits(ctx,1503,1,detect_flag1);
            qDebug()<<"after:"<<detect_flag1[0];
            
            }
      }
  });
}

// 开始循环旋转
void modbusSocket::startLoopRotation() {
    // if (!isLoopRotating) {
    //     qDebug() << "=== 开始循环旋转功能 ===";
    //     qDebug() << "设置循环旋转标志为true";
    //     isLoopRotating = true;
    //     isForwardRotation = true;
        
    //     qDebug() << "执行初始位置设置:";
    //     // qDebug() << "  Y轴移动到:" << this->yMAXData;
    //     qDebug() << "  X轴移动到:" << (this->xMidData + 2500);
        
    //     // 首先移动到指定位置
    //     // y_micore(this->yMAXData);
    //     x_basic_move(this->xMidData + 2500);
        
    //     qDebug() << "初始位置设置完成，开始第一次旋转";
    //     // 开始第一次旋转
    //     executeRotationCycle();
    // } else {
    //     qDebug() << "循环旋转已在运行中，忽略重复启动请求";
    // }
    qDebug() << "=== 开始循环旋转功能 ===";
    rotate();  // 正向旋转（函数内部已包含等待旋转完成的逻辑）
    revent();

}

// 停止循环旋转
void modbusSocket::stopLoopRotation() {
    if (isLoopRotating) {
        qDebug() << "=== 停止循环旋转功能 ===";
        qDebug() << "设置循环旋转标志为false";
        isLoopRotating = false;
        rotateTimer->stop();
        qDebug() << "循环旋转定时器已停止";
        qDebug() << "=== 循环旋转功能已停止 ===";
    } else {
        qDebug() << "循环旋转未在运行，忽略停止请求";
    }
}



// 执行一次旋转循环
void modbusSocket::executeRotationCycle() {
    if (!isLoopRotating) {
        qDebug() << "循环旋转已停止，退出执行循环";
        return;
    }
    
    qDebug() << "=== 执行旋转循环 ===";
    qDebug() << "当前旋转方向:" << (isForwardRotation ? "正向" : "反向");
    
    QtConcurrent::run([this]() {
        if (isForwardRotation) {
            qDebug() << "开始执行正向旋转";
            rotate();  // 正向旋转（函数内部已包含等待旋转完成的逻辑）
            
            if (isLoopRotating) {  // 检查是否仍在循环中
                qDebug() << "正向旋转完成，等待1秒...";
                std::this_thread::sleep_for(std::chrono::seconds(1));
                qDebug() << "1秒等待完成，切换到反向旋转";
                isForwardRotation = false;
                
                // 1秒后执行反向旋转
                if (isLoopRotating) {
                    QMetaObject::invokeMethod(this, "executeRotationCycle", Qt::QueuedConnection);
                }
            }
        } else {
            qDebug() << "开始执行反向旋转";
            revent();  // 反向旋转（函数内部已包含等待旋转完成的逻辑）
            
            if (isLoopRotating) {  // 检查是否仍在循环中
                qDebug() << "反向旋转完成，等待1秒...";
                std::this_thread::sleep_for(std::chrono::seconds(1));
                qDebug() << "1秒等待完成，切换到正向旋转";
                isForwardRotation = true;
                
                // 1秒后执行正向旋转
                if (isLoopRotating) {
                    QMetaObject::invokeMethod(this, "executeRotationCycle", Qt::QueuedConnection);
                }
            }
        }
        qDebug() << "=== 旋转循环执行完成 ===";
    });
}

// 添加新的测试函数实现
void modbusSocket::testDetectFinally(){
    qDebug() << "=== 测试检测完成信号(testDetectFinally) ===";
    qDebug() << "写入地址:" << this->detect_finally_address << "值: 1";
    this->value = 1;
    rc = modbus_write_bit(this->ctx, this->detect_finally_address, this->value);
    if(rc == -1){
        qDebug()<<"检测完成信号写入失败! 地址:" << this->detect_finally_address;
    } else {
        qDebug()<<"检测完成信号写入成功! 地址:" << this->detect_finally_address;
    }
    qDebug() << "=== 检测完成信号测试完成 ===";
}

void modbusSocket::testDetectRes(){
    qDebug() << "=== 测试检测结果信号(testDetectRes) ===";
    qDebug() << "写入地址:" << this->detect_res_address << "值: 1";
    this->value = 1;
    rc = modbus_write_bit(this->ctx, this->detect_res_address, this->value);
    if(rc == -1){
        qDebug()<<"检测结果信号写入失败! 地址:" << this->detect_res_address;
    } else {
        qDebug()<<"检测结果信号写入成功! 地址:" << this->detect_res_address;
    }
    qDebug() << "=== 检测结果信号测试完成 ===";
}

// 添加测试写入0的函数实现
void modbusSocket::testDetectFinallyZero(){
    qDebug() << "=== 测试检测完成信号写入0(testDetectFinallyZero) ===";
    qDebug() << "写入地址:" << this->detect_finally_address << "值: 0";
    this->value = 0;
    rc = modbus_write_bit(this->ctx, this->detect_finally_address, this->value);
    if(rc == -1){
        qDebug()<<"检测完成信号(0)写入失败! 地址:" << this->detect_finally_address;
    } else {
        qDebug()<<"检测完成信号(0)写入成功! 地址:" << this->detect_finally_address;
    }
    qDebug() << "=== 检测完成信号(0)测试完成 ===";
}

void modbusSocket::testDetectResZero(){
    qDebug() << "=== 测试检测结果信号写入0(testDetectResZero) ===";
    qDebug() << "写入地址:" << this->detect_res_address << "值: 0";
    this->value = 0;
    rc = modbus_write_bit(this->ctx, this->detect_res_address, this->value);
    if(rc == -1){
        qDebug()<<"检测结果信号(0)写入失败! 地址:" << this->detect_res_address;
    } else {
        qDebug()<<"检测结果信号(0)写入成功! 地址:" << this->detect_res_address;
    }
    qDebug() << "=== 检测结果信号(0)测试完成 ===";
}

// 训练顺逆旋转功能（不移动Y轴）
void modbusSocket::startTrainingRotation() {
    if (!isLoopRotating) {
        qDebug() << "=== 开始训练顺逆旋转功能 ===";
        qDebug() << "设置训练旋转标志为true";
        isLoopRotating = true;
        isForwardRotation = true;
        
        // 不移动Y轴，直接开始旋转
        qDebug() << "开始第一次训练旋转（不移动Y轴）";
        executeTrainingRotationCycle();
    } else {
        qDebug() << "训练旋转已在运行中，忽略重复启动请求";
    }
}

void modbusSocket::stopTrainingRotation() {
    if (isLoopRotating) {
        qDebug() << "=== 停止训练顺逆旋转功能 ===";
        qDebug() << "设置训练旋转标志为false";
        isLoopRotating = false;
        rotateTimer->stop();
        qDebug() << "训练旋转定时器已停止";
        qDebug() << "=== 训练顺逆旋转功能已停止 ===";
    } else {
        qDebug() << "训练旋转未在运行，忽略停止请求";
    }
}

void modbusSocket::executeTrainingRotationCycle() {
    if (!isLoopRotating) {
        qDebug() << "训练旋转已停止，退出执行循环";
        return;
    }
    
    qDebug() << "=== 执行训练旋转循环 ===";
    qDebug() << "当前旋转方向:" << (isForwardRotation ? "正向" : "反向");
    
    QtConcurrent::run([this]() {
        if (isForwardRotation) {
            qDebug() << "开始执行正向旋转";
            rotate();  // 正向旋转
            
            if (isLoopRotating) {
                qDebug() << "正向旋转完成，等待1秒...";
                std::this_thread::sleep_for(std::chrono::seconds(1));
                qDebug() << "1秒等待完成，切换到反向旋转";
                isForwardRotation = false;
                
                if (isLoopRotating) {
                    QMetaObject::invokeMethod(this, "executeTrainingRotationCycle", Qt::QueuedConnection);
                }
            }
        } else {
            qDebug() << "开始执行反向旋转";
            revent();  // 反向旋转
            
            if (isLoopRotating) {
                qDebug() << "反向旋转完成，等待1秒...";
                std::this_thread::sleep_for(std::chrono::seconds(1));
                qDebug() << "1秒等待完成，切换到正向旋转";
                isForwardRotation = true;
                
                if (isLoopRotating) {
                    QMetaObject::invokeMethod(this, "executeTrainingRotationCycle", Qt::QueuedConnection);
                }
            }
        }
        qDebug() << "=== 训练旋转循环执行完成 ===";
    });
}

// 清料操作
void modbusSocket::clearMaterial() {
    qDebug() << "=== 执行清料操作(clearMaterial) ===";
    qDebug() << "清料地址: 105, 值: 1";
    this->value = 1;
    rc = modbus_write_bit(this->ctx, 105, this->value);
    if(rc == -1){
        qDebug()<<"清料操作写入失败! 地址: 105";
    } else {
        qDebug()<<"清料操作写入成功! 地址: 105";
    }
    // 等待清料完成
    std::this_thread::sleep_for(std::chrono::seconds(3));
    qDebug() << "=== 清料操作完成 ===";
}

// 换料操作
void modbusSocket::changeMaterial() {
    qDebug() << "=== 执行换料操作(changeMaterial) ===";
    qDebug() << "换料地址: 106, 值: 1";
    this->value = 1;
    rc = modbus_write_bit(this->ctx, 106, this->value);
    if(rc == -1){
        qDebug()<<"换料操作写入失败! 地址: 106";
    } else {
        qDebug()<<"换料操作写入成功! 地址: 106";
    }
    // 等待换料完成
    std::this_thread::sleep_for(std::chrono::seconds(5));
    qDebug() << "=== 换料操作完成 ===";
}

// 自动录制主流程
void modbusSocket::performAutoRecording() {
    qDebug() << "=== 开始自动录制流程 ===";
    qDebug() << "将录制" << this->autoRecording_totalDiscs << "张盘片";
    
    // 第一个盘片开始前的初始化操作
    // qDebug() << "执行初始化操作：复位 -> 清料 -> 换料";
    // reset();
    // std::this_thread::sleep_for(std::chrono::seconds(2));
    // clearMaterial();
    // changeMaterial();
    // qDebug() << "执行初始化操作：开始";
    // begin();
    // std::this_thread::sleep_for(std::chrono::seconds(2));
    // 循环录制指定数量的盘片
    for (int discNum = 1; discNum <= this->autoRecording_totalDiscs; discNum++) {
        qDebug() << "=== 开始录制第" << discNum << "张盘片 ===";
        recordSingleDisc(discNum);
        qDebug() << "=== 第" << discNum << "张盘片录制完成 ===";
        
        // 发送检测完成信号
        testDetectFinally();
        
        // 如果不是最后一张盘片，等待一段时间再继续
        if (discNum < this->autoRecording_totalDiscs) {
            std::this_thread::sleep_for(std::chrono::seconds(35));
        }
    }
    
    qDebug() << "=== 自动录制流程完成，共录制" << this->autoRecording_totalDiscs << "张盘片 ===";
    emit autoRecordingComplete();
}

// 录制单张盘片
void modbusSocket::recordSingleDisc(int discNumber) {
    qDebug() << "=== 录制第" << discNumber << "张盘片 ===";
    
    // 使用配置文件中的位置参数
    int x1 = this->autoRecording_x1;  // min位置
    int x2 = this->autoRecording_x2;  // mid位置  
    int x3 = this->autoRecording_x3;  // max位置
    int x4 = this->autoRecording_x4;  // 第四个X轴位置
    int y = this->autoRecording_y;    // Y轴位置
    int z = this->autoRecording_z;    // Z轴位置
    
    qDebug() << "使用配置参数: x1=" << x1 << " x2=" << x2 << " x3=" << x3 << " x4=" << x4 << " y=" << y << " z=" << z;
    
    // 设置Y轴位置
    qDebug() << "设置Y轴位置到:" << y;
    y_micore(y);
    
    // 设置Z轴位置
    qDebug() << "设置Z轴位置到:" << z;
    z_mobile(z);
    
    // 第一个位置：x1 (min)
    qDebug() << "移动到第一个位置 x1:" << x1;
    recordAtPosition(x1, QString::number(discNumber) + "_x1");
    
    // 第二个位置：x2 (mid)
    qDebug() << "移动到第二个位置 x2:" << x2;
    recordAtPosition(x2, QString::number(discNumber) + "_x2");
    
    // 第三个位置：x3 (max)
    qDebug() << "移动到第三个位置 x3:" << x3;
    recordAtPosition(x3, QString::number(discNumber) + "_x3");

    // 第四个位置：x4 (第四个X轴位置)
    qDebug() << "移动到第四个位置 x4:" << x4;
    recordAtPosition(x4, QString::number(discNumber) + "_x4");
    
    emit discRecordingComplete(discNumber);
}

// 在指定位置录制
void modbusSocket::recordAtPosition(int xPosition, const QString& suffix) {
    qDebug() << "=== 在位置" << xPosition << "录制，后缀:" << suffix << " ===";
    
    // 首先移动到指定X轴位置（不录制移动过程）
    qDebug() << "移动X轴到目标位置" << xPosition << "（不录制移动过程）";
    x_basic_move(xPosition);
    qDebug() << "X轴移动完成，当前位置:" << xPosition;
    
    // 移动Z轴到配置的位置
    qDebug() << "移动Z轴到配置位置" << this->autoRecording_z << "（不录制移动过程）";
    z_mobile(this->autoRecording_z);
    qDebug() << "Z轴移动完成，当前位置:" << this->autoRecording_z;
    
    // 构造完整的录制路径信息，包含自动录制的基础路径
    QString fullSuffix = QString("auto_%1").arg(suffix);
    qDebug() << "构造的完整后缀:" << fullSuffix;
    
    // 获取录制配置实例
    auto recordingConfig = RecordingConfig::getInstance();
    
    // 保存当前的基础路径
    QString originalBasePath = recordingConfig->getBasePath();
    qDebug() << "保存原始基础路径:" << originalBasePath;
    
    // 设置自动录制的基础路径
    recordingConfig->setBasePath(this->autoRecording_basePath);
    qDebug() << "设置自动录制基础路径为:" << this->autoRecording_basePath;
    
    // 在开始正向旋转前启动录制
    qDebug() << "准备开始旋转，发送录制开始信号，后缀:" << fullSuffix;
    emit startRecord(fullSuffix);
    
    // 等待录制系统启动
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    qDebug() << "录制系统已启动，开始旋转过程";
    
    // 执行正反旋转（录制整个旋转过程）
    qDebug() << "开始正向旋转（录制中）";
    rotate();   // 正向旋转
    qDebug() << "正向旋转完成，开始反向旋转（录制中）";
    revent();   // 反向旋转
    qDebug() << "反向旋转完成，旋转过程结束";
    
    // 旋转完成后停止录制
    qDebug() << "旋转过程完成，发送录制停止信号";
    emit stopRecord();
    
    // 等待录制系统完全停止
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    qDebug() << "录制系统已停止";
    
    // 恢复原来的基础路径
    recordingConfig->setBasePath(originalBasePath);
    qDebug() << "恢复原始基础路径:" << originalBasePath;
    
    qDebug() << "=== 位置" << xPosition << "录制完成 ===";
}

// 加载自动录制配置
void modbusSocket::loadAutoRecordingConfig() {
    qDebug() << "=== 加载自动录制配置 ===";
    
    QString configPath = ConfigManager::getInstance().getConfigPath();
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法打开配置文件" << configPath << "，使用默认配置";
        return;
    }
    
    QByteArray jsonData = file.readAll();
    QJsonDocument document = QJsonDocument::fromJson(jsonData);
    
    if (document.isNull() || !document.isObject()) {
        qDebug() << "配置文件格式错误，使用默认配置";
        return;
    }
    
    QJsonObject rootObj = document.object();
    
    if (rootObj.contains("autoRecording") && rootObj["autoRecording"].isObject()) {
        QJsonObject autoRecordingObj = rootObj["autoRecording"].toObject();
        
        // 读取盘片总数
        if (autoRecordingObj.contains("totalDiscs") && autoRecordingObj["totalDiscs"].isDouble()) {
            this->autoRecording_totalDiscs = autoRecordingObj["totalDiscs"].toInt();
        }
        
        // 读取基础路径
        if (autoRecordingObj.contains("basePath") && autoRecordingObj["basePath"].isString()) {
            this->autoRecording_basePath = autoRecordingObj["basePath"].toString();
        }
        
        // 读取位置参数
        if (autoRecordingObj.contains("positions") && autoRecordingObj["positions"].isObject()) {
            QJsonObject positionsObj = autoRecordingObj["positions"].toObject();
            
            if (positionsObj.contains("x1") && positionsObj["x1"].isDouble()) {
                this->autoRecording_x1 = positionsObj["x1"].toInt();
            }
            if (positionsObj.contains("x2") && positionsObj["x2"].isDouble()) {
                this->autoRecording_x2 = positionsObj["x2"].toInt();
            }
            if (positionsObj.contains("x3") && positionsObj["x3"].isDouble()) {
                this->autoRecording_x3 = positionsObj["x3"].toInt();
            }
            if (positionsObj.contains("x4") && positionsObj["x4"].isDouble()) {
                this->autoRecording_x4 = positionsObj["x4"].toInt();
            }
            if (positionsObj.contains("y") && positionsObj["y"].isDouble()) {
                this->autoRecording_y = positionsObj["y"].toInt();
            }
            if (positionsObj.contains("z") && positionsObj["z"].isDouble()) {
                this->autoRecording_z = positionsObj["z"].toInt();
            }
        }
        
        // 读取旋转速度
        if (autoRecordingObj.contains("rotationSpeed") && autoRecordingObj["rotationSpeed"].isDouble()) {
            this->rotate_speed = autoRecordingObj["rotationSpeed"].toInt();
        }
        
        qDebug() << "自动录制配置加载成功:";
        qDebug() << "  盘片总数:" << this->autoRecording_totalDiscs;
        qDebug() << "  基础路径:" << this->autoRecording_basePath;
        qDebug() << "  位置参数: x1=" << this->autoRecording_x1 << " x2=" << this->autoRecording_x2 << " x3=" << this->autoRecording_x3 << " x4=" << this->autoRecording_x4 << " y=" << this->autoRecording_y << " z=" << this->autoRecording_z;
        qDebug() << "  旋转速度:" << this->rotate_speed;
    } else {
        qDebug() << "配置文件中未找到autoRecording配置，使用默认值";
    }
    
    qDebug() << "=== 自动录制配置加载完成 ===";
}


// X1-X4位置的录制功能实现
void modbusSocket::recordAtX1() {
    qDebug() << "=== X1位置录制功能 ===";
    
    // 设置Y轴和Z轴到配置位置
    y_micore(this->autoRecording_y);
    z_mobile(this->autoRecording_z);
    
    // 移动到X1位置
    x_basic_move(this->autoRecording_x1);
    
    // 开始录制
    emit startRecord("manual_x1");
    
    // 等待录制系统启动
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // 执行旋转录制
    rotate();   // 正向旋转
    revent();   // 反向旋转
    
    // 停止录制
    emit stopRecord();
    
    qDebug() << "=== X1位置录制完成 ===";
}

void modbusSocket::recordAtX2() {
    qDebug() << "=== X2位置录制功能 ===";
    
    // 设置Y轴和Z轴到配置位置
    y_micore(this->autoRecording_y);
    z_mobile(this->autoRecording_z);
    
    // 移动到X2位置
    x_basic_move(this->autoRecording_x2);
    
    // 开始录制
    emit startRecord("manual_x2");
    
    // 等待录制系统启动
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // 执行旋转录制
    rotate();   // 正向旋转
    revent();   // 反向旋转
    
    // 停止录制
    emit stopRecord();
    
    qDebug() << "=== X2位置录制完成 ===";
}

void modbusSocket::recordAtX3() {
    qDebug() << "=== X3位置录制功能 ===";
    
    // 设置Y轴和Z轴到配置位置
    y_micore(this->autoRecording_y);
    z_mobile(this->autoRecording_z);
    
    // 移动到X3位置
    x_basic_move(this->autoRecording_x3);
    
    // 开始录制
    emit startRecord("manual_x3");
    
    // 等待录制系统启动
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // 执行旋转录制
    rotate();   // 正向旋转
    revent();   // 反向旋转
    
    // 停止录制
    emit stopRecord();
    
    qDebug() << "=== X3位置录制完成 ===";
}

void modbusSocket::recordAtX4() {
    qDebug() << "=== X4位置录制功能 ===";
    
    // 设置Y轴和Z轴到配置位置
    y_micore(this->autoRecording_y);
    z_mobile(this->autoRecording_z);
    
    // 移动到X4位置
    x_basic_move(this->autoRecording_x4);
    
    // 开始录制
    emit startRecord("manual_x4");
    
    // 等待录制系统启动
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // 执行旋转录制
    rotate();   // 正向旋转
    revent();   // 反向旋转
    
    // 停止录制
    emit stopRecord();
    
    qDebug() << "=== X4位置录制完成 ===";
}

// X1-X4位置的移动功能实现（不录制）
void modbusSocket::moveToX1() {
    qDebug() << "=== 移动到X1位置 ===";
    
    // 设置Y轴和Z轴到配置位置
    y_micore(this->autoRecording_y);
    z_mobile(this->autoRecording_z);
    
    // 移动到X1位置
    x_basic_move(this->autoRecording_x1);
    
    // 执行旋转（不录制）
    rotate();   // 正向旋转
    revent();   // 反向旋转
    
    qDebug() << "=== X1位置移动完成 ===";
}

void modbusSocket::moveToX2() {
    qDebug() << "=== 移动到X2位置 ===";
    
    // 设置Y轴和Z轴到配置位置
    y_micore(this->autoRecording_y);
    z_mobile(this->autoRecording_z);
    
    // 移动到X2位置
    x_basic_move(this->autoRecording_x2);
    
    // 执行旋转（不录制）
    rotate();   // 正向旋转
    revent();   // 反向旋转
    
    qDebug() << "=== X2位置移动完成 ===";
}

void modbusSocket::moveToX3() {
    qDebug() << "=== 移动到X3位置 ===";
    
    // 设置Y轴和Z轴到配置位置
    y_micore(this->autoRecording_y);
    z_mobile(this->autoRecording_z);
    
    // 移动到X3位置
    x_basic_move(this->autoRecording_x3);
    
    // 执行旋转（不录制）
    rotate();   // 正向旋转
    revent();   // 反向旋转
    
    qDebug() << "=== X3位置移动完成 ===";
}

void modbusSocket::moveToX4() {
    qDebug() << "=== 移动到X4位置 ===";
    
    // 设置Y轴和Z轴到配置位置
    y_micore(this->autoRecording_y);
    z_mobile(this->autoRecording_z);
    
    // 移动到X4位置
    x_basic_move(this->autoRecording_x4);
    
    // 执行旋转（不录制）
    rotate();   // 正向旋转
    revent();   // 反向旋转
    
    qDebug() << "=== X4位置移动完成 ===";
}

