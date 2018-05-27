#include "nbiot.h"
#include "stm32f10x.h"
#include "usart.h"	 
#include "crc16.h"
#include "string.h"

extern u16 USART2_RX_STA;//在usart.c中定义
extern u8 USART2_RX_BUF[USART_REC_LEN];//在usart.c中定义

//通过串口2发送AT指令给NB-IoT模块
void SendData(u16 sdata)
{
	int i = 0;
	char t[50];
	char data[10];
	char datastr[21];
	char res[5];
	
	//初始化t
	for(i = 0; i < 50; i++)
	{
		t[i] = '\0';
	}
	
	//准备数据
	data[0] = 0x01;//从设备地址
	data[1] = 0x46;//寄存器单元类型
	data[2] = 0x00;//寄存器单元长度
	data[3] = 0x00;
	data[4] = 0x00;
	data[5] = 0x01;
	data[6] = 0x02;//数据长度
	data[7] = (u8)((sdata & 0xFF00)>> 8);//数据
	data[8] = (u8)(sdata & 0x00FF);
	
	//获取字节数据的字符串形式
	getString(data, 9, datastr);
	//获取字节数据的crc16校验码
	getCrc16(data, 9, res);
	
	//字符串拼接
	strcpy(t, "AT+NMGS=11,");
	strcat(t, datastr);
	strcat(t, res);
	strcat(t, "\r\n");
	
	//发送
	Usart_SendString(USART2, t);
}

//发送AT指令,发送成功返回1，发送失败返回-1
int sendATCmd(char * cmd)
{
	int j = 0, flag = 0;
	do
	{
		Usart_SendString(USART2, cmd);
		//等待接收成功后回应的结果
		delay_ms(3000);
		printf("send status:");
		//判断是否发送成功
		while(j < USART2_RX_STA)
		{
			printf("%c", USART2_RX_BUF[j]);
			if((USART2_RX_BUF[j-1] == 'O' || USART2_RX_BUF[j-1] == 'o') && (USART2_RX_BUF[j] == 'K' || USART2_RX_BUF[j] == 'k'))
			{
				flag = 1;//发送成功
				USART2_RX_STA = 0;
				j = 0;
				break;
			}
			if(((USART2_RX_BUF[j-1] == 'O' || USART2_RX_BUF[j-1] == 'o') && (USART2_RX_BUF[j] == 'R' || USART2_RX_BUF[j] == 'r')))
			{
				flag = -1;//发送失败
				USART2_RX_STA = 0;
				j = 0;
				break;
			}
			j++;
		}
		
		printf("\r\n");
	}while(flag == 0);
	return flag;
}

//进入临时指令模式
void IntoTempCmdMode()
{
	int j = 0, flag = 0;
	
	//确保退出临时指令模式
	while(sendATCmd("AT+ENTM\r\n") == 1);
	
	do
	{
		Usart_SendString(USART2, "+++");
		delay_ms(1000);
		
		//判断是否发送成功
		while(j < USART2_RX_STA)
		{
			if(USART2_RX_BUF[j] == 'a')
			{
				USART2_RX_STA = 0;
				j = 0;
				break;
			}
			j++;
		}
		
		Usart_SendString(USART2, "a");
		delay_ms(1000);
		//判断是否发送成功
		while(j < USART2_RX_STA)
		{
			if((USART2_RX_BUF[j-1] == 'O' || USART2_RX_BUF[j-1] == 'o') && (USART2_RX_BUF[j] == 'K' || USART2_RX_BUF[j] == 'k'))
			{
				flag = 1;//发送成功
				USART2_RX_STA = 0;
				j = 0;
				break;
			}
			j++;
		}
	}while(flag == 0);
	printf("NB模块进入临时指令模式...\r\n");
}

/*
NB-IoT模块接收到并解析AT指令后将数据发送到服务器。
当设备无法正确发送数据时，需要通过AT+Z指令将设备重新启动。
需要注意的是，所有的AT指令都需要进入指令模式才可以被识别运行。
设备从CoAP模式切换到临时指令模式的时序为：
1．	向设备发送“+++”，模块接收到后会回复一个“a”；
2．	在3秒之内再向模块发送一个‘a’；
3．	模块接收到后，回复“+ok”，代表已经进入临时指令模式。
*/
void NB_Init()
{
	int i = 0;
	//进入临时指令模式
	IntoTempCmdMode();
	
	//重启
	while(sendATCmd("AT+Z\r\n") == -1);
	
	printf("NB模块重新启动成功...\r\n");
}
