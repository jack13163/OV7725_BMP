#ifndef NB_IOT_H
#define NB_IOT_H
#include "stm32f10x.h"

void NB_Init();

//发送AT指令,发送成功返回1，发送失败返回-1
int sendATCmd(char * cmd);

//通过串口2发送AT指令给NB-IoT模块
void SendData(u16 sdata);

#endif