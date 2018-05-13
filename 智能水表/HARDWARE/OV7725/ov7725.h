#ifndef _OV7725_H
#define _OV7725_H
#include "sys.h"
#include "sccb.h"
//////////////////////////////////////////////////////////////////////////////////
//ALIENTEK战舰STM32开发板
//OV7725 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2017/11/1
//版本：V1.0		    							    							  
//////////////////////////////////////////////////////////////////////////////////


#define OV7725_MID				0X7FA2    
#define OV7725_PID				0X7721

#define OV7725_VSYNC  	PAin(8)			//同步信号检测IO
#define OV7725_WRST		PDout(6)		//写指针复位
#define OV7725_WREN		PBout(3)		//写入FIFO使能
#define OV7725_RCK_H	GPIOB->BSRR=1<<4//设置读数据时钟高电平
#define OV7725_RCK_L	GPIOB->BRR=1<<4	//设置读数据时钟低电平
#define OV7725_RRST		PGout(14)  		//读指针复位
#define OV7725_CS		PGout(15)  		//片选信号(OE)
								  					 
#define OV7725_DATA   	GPIO_ReadInputData(GPIOC,0x00FF) 	//数据输入端口



	    				 
u8   OV7725_Init(void);		  	   		 
void OV7725_Light_Mode(u8 mode);
void OV7725_Color_Saturation(s8 sat);
void OV7725_Brightness(s8 bright);
void OV7725_Contrast(s8 contrast);
void OV7725_Special_Effects(u8 eft);
void OV7725_Window_Set(u16 width,u16 height,u8 mode);
#endif





















