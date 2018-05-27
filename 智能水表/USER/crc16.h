#ifndef CRC16_H
#define CRC16_H
#include "stm32f10x.h"

/* 将字节数组转换为字符串
* data：字节数组
* str：字符数组（结果）
*/
void getString(u8 *data, int len, char *str);

/* 获取crc16检验后的数据帧
* data:已经填充好的数据帧，需要计算校验码
* crcstr:crc16检验后的生成的字符串数组
*/
void getCrc16(u8 *data, int len, char* crcstr);



#endif