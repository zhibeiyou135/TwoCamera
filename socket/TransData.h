#include <iostream>
#include "string.h"
#include <QObject>
#ifndef TransData_H
#define TransData_H



extern int socket_cfd;
extern char recv_buff[15];
extern char send_buff[15];
extern int saveflag;
extern int clearflag;
extern QString time_path;
extern int resultflag;
extern QString hezi_name;
extern QString panpian_name;
extern int moniFlag;
extern int resDataQueFlag;

#endif