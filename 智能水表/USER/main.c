#include "delay.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"	 
#include "piclib.h"
#include "ov7725.h"
#include "key.h"
#include "exti.h"
#include "led.h"
#include "imageprocess.h"

extern u8 ov_sta;	//在exit.c里面定义
extern u16 USART2_RX_STA;//在usart.c中定义
extern u8 USART2_RX_BUF[USART_REC_LEN];//在usart.c中定义
extern u8 data[11];//在crc16.c中定义
extern char res[22];//在crc16.c中定义

//由于OV7725传感器安装方式原因,OV7725_WINDOW_WIDTH相当于LCD的高度，OV7725_WINDOW_HEIGHT相当于LCD的宽度
//注意：此宏定义只对OV7725有效
#define  OV7725_WINDOW_WIDTH		320 // <=320
#define  OV7725_WINDOW_HEIGHT		240 // <=240

FRESULT res_sd;//文件操作结果
FIL fnew; //文件
UINT fnum; //文件成功读写数量

//更新LCD显示
void camera_refresh(void)
{
	u32 i,j;
 	u16 color;
	BITMAPINFO bmp;
	
	//按下S1拍摄图片
	if(ov_sta && KEY_Scan(1) == S1)
	{
		//打开文件，若不存在就创建
		res_sd = f_open(&fnew, "0:test1.bmp", FA_OPEN_ALWAYS | FA_WRITE);
		
		//文件打开成功
		if(res_sd == FR_OK)
		{
			//填写文件信息头信息  
			bmp.bmfHeader.bfType = 0x4D42;				//bmp类型  
			bmp.bmfHeader.bfOffBits=sizeof(bmp.bmfHeader) + sizeof(bmp.bmiHeader) + sizeof(bmp.RGB_MASK);						//位图信息结构体所占的字节数
			bmp.bmfHeader.bfSize= bmp.bmfHeader.bfOffBits + 320*240*2;	//文件大小（信息结构体+像素数据）
			bmp.bmfHeader.bfReserved1 = 0x0000;		//保留，必须为0
			bmp.bmfHeader.bfReserved2 = 0x0000;  			
			
			//填写位图信息头信息  
			bmp.bmiHeader.biSize=sizeof(bmp.bmiHeader);  				    //位图信息头的大小
			bmp.bmiHeader.biWidth=320;  														//位图的宽度
			bmp.bmiHeader.biHeight=240;  			    //图像的高度
			bmp.bmiHeader.biPlanes=1;  				    //目标设别的级别，必须是1
			bmp.bmiHeader.biBitCount=16;          //每像素位数
			bmp.bmiHeader.biCompression=3;  	    //每个象素的比特由指定的掩码（RGB565掩码）决定。  (非常重要)
			bmp.bmiHeader.biSizeImage=320*240*2;  //实际位图所占用的字节数（仅考虑位图像素数据）
			bmp.bmiHeader.biXPelsPerMeter=0;			//水平分辨率
			bmp.bmiHeader.biYPelsPerMeter=0; 			//垂直分辨率
			bmp.bmiHeader.biClrImportant=0;   	  //说明图像显示有重要影响的颜色索引数目，0代表所有的颜色一样重要
			bmp.bmiHeader.biClrUsed=0;  			    //位图实际使用的彩色表中的颜色索引数，0表示使用所有的调色板项
			
			//RGB565格式掩码
			bmp.RGB_MASK[0] = 0X00F800;
			bmp.RGB_MASK[1] = 0X0007E0;
			bmp.RGB_MASK[2] = 0X00001F;
			
			//写文件头进文件  
			res_sd= f_write(&fnew, &bmp, sizeof(bmp), &fnum);
		
			//读指针复位
			OV7725_RRST=0;				//开始复位读指针
			OV7725_RCK_L;
			OV7725_RCK_H;
			OV7725_RCK_L;
			OV7725_RRST=1;				//复位读指针结束 
			OV7725_RCK_H; 
			
			/*图像花屏的原因在于读取时的干扰和读取时漏掉几个像素*/
			for(i=0;i<240;i++)
			{
				for(j=0;j<320;j++)
				{
					OV7725_RCK_L;
					color=GPIOC->IDR&0XFF;	//读数据
					OV7725_RCK_H; 
					color<<=8;  
					OV7725_RCK_L;
					color|=GPIOC->IDR&0XFF;	//读数据
					OV7725_RCK_H; 
					
					//写位图信息头进内存卡
					f_write(&fnew, &color, sizeof(color), &fnum);
				}
			}
		}
				
		//关闭文件
		f_close(&fnew);
	}
	
	//没有按键按下，刷新LCD
	if(ov_sta)
	{
		LCD_Scan_Dir(U2D_L2R);		//从上到下,从左到右 
		LCD_WriteRAM_Prepare();   //开始写入GRAM	
		
		//读指针复位
		OV7725_RRST=0;				//开始复位读指针 
		OV7725_RCK_L;
		OV7725_RCK_H;
		OV7725_RCK_L;
		OV7725_RRST=1;				//复位读指针结束 
		OV7725_RCK_H; 
		
		/*图像花屏的原因在于读取时的干扰和读取时漏掉几个像素*/
		for(i=0;i<240;i++)
		{
			for(j=0;j<320;j++)
			{
				OV7725_RCK_L;
				color=GPIOC->IDR&0XFF;	//读数据
				OV7725_RCK_H; 
				color<<=8;  
				OV7725_RCK_L;
				color|=GPIOC->IDR&0XFF;	//读数据
				OV7725_RCK_H; 
				LCD->LCD_RAM=color; 
			}
		}
		ov_sta=0;					//开始下一次采集
		LCD_Scan_Dir(DFT_SCAN_DIR);	//恢复默认扫描方向 
	}
}

//通过串口2发送AT指令给NB-IoT模块
void SendData()
{
	data[0] = 0x01;
	data[1] = 0x46;
	data[2] = 0x00;
	data[3] = 0x00;
	data[4] = 0x00;
	data[5] = 0x01;
	data[6] = 0x02;
	data[7] = 0x00;//数据
	data[8] = 0x12;
				
	getCrc16(data, res);
				
	Usart_SendString(USART2, "AT+NMGS=11,");
	Usart_SendString(USART2, res);
	Usart_SendString(USART2, "\r\n");
}

int main(void)
{
	u8 lightmode=0,saturation=2,brightness=2,contrast=2,effect=0;
	u8 i = 0, j = 0;
	int res[5];//识别结果
	char r[6];
	u8 flag = 0;
	
	delay_init();	    	//延时函数初始化	  
	uart_init(9600);	 	//串口初始化为9600
	usart2_init(115200);
	LCD_Init();					//初始化LCD液晶显示屏
	KEY_Init();					//按键初始化
 	mem_init(SRAMIN);		//初始化内部内存池(很重要)
 	exfuns_init();			//为fatfs相关变量申请内存  
  f_mount(0,fs[0]); 	//挂载SD卡 
	piclib_init();			//初始化画图

	while(OV7725_Init() != 0);				//初始化OV7725摄像头
	
	POINT_COLOR = RED;
	LCD_ShowString(60,210,200,16,16,"OV7725 Init OK");
	//特效
  OV7725_Light_Mode(lightmode);
	OV7725_Color_Saturation(saturation);
	OV7725_Brightness(brightness);
	OV7725_Contrast(contrast);
	OV7725_Special_Effects(effect);
	
	//设置输出格式
	OV7725_Window_Set(OV7725_WINDOW_WIDTH,OV7725_WINDOW_HEIGHT,0);//QVGA模式输出
	//输出使能
	OV7725_CS=0;
	
	EXTI8_Init();				//使能定时器捕获
	
	LCD_Clear(BLACK);
	while(1)
	{
		camera_refresh();//更新显示
		
		//按下S2显示图片
		if(KEY_Scan(1) == S2)
		{
			LCD_Clear(BLACK);
			ai_load_picfile("0:test1.bmp",0,0,lcddev.width,lcddev.height,1);//显示图片
			delay_ms(5000);
			LCD_Clear(BLACK);//清屏之后可以防止出现割屏现象
			continue;
		}
		
		//按下S3图片处理
		if(KEY_Scan(1) == S3)
		{
			LCD_Clear(WHITE);
			LCD_ShowString(60,210,200,16,16,"Graying......");
			
			//图像灰度化
			Graying("0:test1.bmp", "0:test2.bmp");
			
			//显示图像处理结果
			LCD_Clear(BLACK);
			ai_load_picfile("0:test2.bmp",0,0,lcddev.width,lcddev.height,1);//显示图片
			delay_ms(5000);
			LCD_Clear(BLACK);//清屏之后可以防止出现割屏现象

			LCD_Clear(WHITE);
			LCD_ShowString(60,210,200,16,16,"Ostu......");
			
			//图像二值化
			Ostu("0:test2.bmp", "0:test3.bmp");
			//显示图像处理结果
			LCD_Clear(BLACK);
			ai_load_picfile("0:test3.bmp",0,0,lcddev.width,lcddev.height,1);//显示图片
			delay_ms(5000);
			LCD_Clear(BLACK);//清屏之后可以防止出现割屏现象
			
			//图像分割(图片文件夹的路径为0:PICS/)
			//...
			
			LCD_Clear(WHITE);
			LCD_ShowString(60,210,200,16,16,"Image Recognition......");
			//图像识别
			res[0] = BP_Recongnization("0:PICS/0.bmp");
			res[1] = BP_Recongnization("0:PICS/1.bmp");
			res[2] = BP_Recongnization("0:PICS/2.bmp");
			res[3] = BP_Recongnization("0:PICS/3.bmp");
			res[4] = BP_Recongnization("0:PICS/4.bmp");
			
			r[0] = res[0] + '0';
			r[1] = res[1] + '0';
			r[2] = res[2] + '0';
			r[3] = res[3] + '0';
			r[4] = res[4] + '0';
			r[5] = '\0';
			//输出识别结果
			LCD_Clear(WHITE);
			LCD_ShowString(60,210,200,16,16, r);
			printf("识别结果:%d%d%d%d%d", res[0], res[1], res[2], res[3], res[4]);
			
			//将处理后的数字发送到NB-IoT模块
			do
			{
				//发送AT指令
				SendData();
				
				//等待接收成功后回应的结果
				delay_ms(500);
				
				//判断是否发送成功
				while(j < USART2_RX_STA)
				{
					if((USART2_RX_BUF[j] == 'O' || USART2_RX_BUF[j] == 'o') && (USART2_RX_BUF[j+1] == 'K' || USART2_RX_BUF[j+1] == 'k'))
					{
						flag = 1;//发送成功
						USART2_RX_STA = 0;
						j = 0;
						break;
					}
					j++;
					
					if(j == USART2_RX_STA)
					{
						USART2_RX_STA = 0;
						j = 0;
						break;
					}
				}
			}while(flag == 0);
			
			continue;
		}

		//按下S4显示图片处理结果
		if(KEY_Scan(1) == S4)
		{
			LCD_Clear(BLACK);
			ai_load_picfile("0:test3.bmp",0,0,lcddev.width,lcddev.height,1);//显示图片
			delay_ms(5000);
			LCD_Clear(BLACK);//清屏之后可以防止出现割屏现象
			continue;
		}
		
		i++;
		if(i==15)//DS0闪烁.
		{
			i=0;
			LED0=!LED0;
		}
	}
}