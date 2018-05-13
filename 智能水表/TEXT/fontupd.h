#ifndef __FONTUPD_H__
#define __FONTUPD_H__	 
#include "sys.h"
					  
//////////////////////////////////////////////////////////////////////////////////

//前面6M被fatfs占用了.
//6M以后紧跟的500K字节,用户可以随便用.
//6M+500K字节以后的字节,被字库占用了,不能动!
//字体信息保存地址,占25个字节,第1个字节用于标记字库是否存在.后续每8个字节一组,分别保存起始地址和文件大小														   
extern u32 FONTINFOADDR;	
//字库信息结构体定义
//用来保存字库基本信息，地址，大小等
__packed typedef struct 
{
	u8 fontok;				//字库存在标志，0XAA，字库正常；其他，字库不存在
	u32 ugbkaddr; 			//unigbk的地址
	u32 ugbksize;			//unigbk的大小	 
	u32 f12addr;			//gbk12地址	
	u32 gbk12size;			//gbk12的大小	 
	u32 f16addr;			//gbk16地址
	u32 gkb16size;			//gbk16的大小	 
}_font_info;																   

extern _font_info ftinfo;	//字库信息结构体

u32 fupd_prog(u16 x,u16 y,u8 size,u32 fsize,u32 pos);//显示更新进度
u8 updata_fontx(u16 x,u16 y,u8 size,u8 *fxpath,u8 fx);//更新指定字库
u8 update_font(u16 x,u16 y,u8 size,u8 src);//更新全部字库
u8 font_init(void);//初始化字库
#endif





















