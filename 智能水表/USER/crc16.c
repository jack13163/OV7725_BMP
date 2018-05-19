#include "stm32f10x.h"

//crc16检验算法
//014600000001020064共九个字节，最后两个字节代表数据，另外还需要多追加两个字节的校验码
//待发送的数据帧（共11个字节）


//将一个字节数据转换为两个字符
char byteToChar(u8 b)
{
	char res[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	
	return (char)res[b];
}

/* 将字节数组转换为字符串
* data：字节数组
* str：字符数组（结果）
*/
void getString(u8 *data, int len, char *str)
{
	u8 i;
	u8 res1,res2;
	for(i = 0; i < len; i++)
	{
		res1 = ((data[i] & 0xF0) >> 4);
		res2 = data[i] & 0x0F;
		
		str[2*i] = byteToChar(res1);
		str[2*i+1] = byteToChar(res2);
	}
	str[2 * len] = '\0';
}


/* 获取crc16检验后的数据帧
* data:已经填充好的数据帧，需要计算校验码
* crcstr:crc16检验后的生成的字符串数组
*/
void getCrc16(u8 *data, int len, char* crcstr)
{
	//定义crc16的生成多项式
	u16 gx = 0xA001;
	u8 i, j;
	u16 crcreg = 0xFFFF;
	u8 d[2];
	
	for(i = 0; i < len; i++)
	{
		crcreg = (u16)(crcreg ^ data[i]);
		for(j = 0; j < 8; j++)
		{
			crcreg = (crcreg & 1) != 0 ? (u16)((crcreg >> 1) ^ gx) : (u16)(crcreg >> 1);;
		}
	}
	
	d[0] = (u8)(crcreg & 0x00FF);
	d[1] = (u8)((crcreg & 0xFF00) >> 8);
	getString(d, 2, crcstr);
}