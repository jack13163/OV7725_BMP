#ifndef __BMP_H__
#define __BMP_H__
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 

//********************************************************************************
//升级说明
//V2.0
//1,将jpeg解码库换成了TJPGD,支持更多的jpg/jpeg文件,支持小尺寸图片快速jpeg显示 
//2,pic_phy里面新增3个函数,用于开窗显示,以提高小尺寸jpg图片的显示速度
//3,ai_load_picfile函数,新增一个参数:fast,用于设置是否使能jpeg/jpg快速显示
//注意:这里的小尺寸是指:jpg/jpeg图片尺寸小于等于LCD尺寸.
//////////////////////////////////////////////////////////////////////////////////
					    
//////////////////////////////////////////用户配置区///////////////////////////////
#define BMP_USE_MALLOC		1 		//定义是否使用malloc,这里我们选择使用malloc
#define BMP_DBUF_SIZE		2048	//定义bmp解码数组的大小(最少应为LCD宽度*3)
//////////////////////////////////////////////END/////////////////////////////////

//BMP信息头
typedef __packed struct
{
    u32 biSize ;		   		//说明BITMAPINFOHEADER结构所需要的字数。
    long  biWidth ;		   	//说明图象的宽度，以象素为单位 
    long  biHeight ;	   	//说明图象的高度，以象素为单位 
    u16  biPlanes ;	   		//为目标设备说明位面数，其值将总是被设为1 
    u16  biBitCount ;	   	//说明比特数/象素，其值为1、4、8、16、24、或32
    u32 biCompression ;  	//说明图象数据压缩的类型。其值可以是下述值之一：
		//BI_RGB：没有压缩；
		//BI_RLE8：每个象素8比特的RLE压缩编码，压缩格式由2字节组成(重复象素计数和颜色索引)；  
    //BI_RLE4：每个象素4比特的RLE压缩编码，压缩格式由2字节组成
  	//BI_BITFIELDS：每个象素的比特由指定的掩码决定。
    u32 biSizeImage ;			//说明图象的大小，以字节为单位。当用BI_RGB格式时，可设置为0  
    long  biXPelsPerMeter ;	//说明水平分辨率，用象素/米表示
    long  biYPelsPerMeter ;	//说明垂直分辨率，用象素/米表示
    u32 biClrUsed ;	  	 	//说明位图实际使用的彩色表中的颜色索引数
    u32 biClrImportant ; 	//说明对图象显示有重要影响的颜色索引的数目，如果是0，表示都重要。 
}BITMAPINFOHEADER ;
//BMP头文件
typedef __packed struct
{
    u16  bfType ;     //文件标志.只对'BM',用来识别BMP位图类型
    u32  bfSize ;	  //文件大小,占四个字节
    u16  bfReserved1 ;//保留
    u16  bfReserved2 ;//保留
    u32  bfOffBits ;  //从文件开始到位图数据(bitmap data)开始之间的的偏移量
}BITMAPFILEHEADER ;
//彩色表 
typedef __packed struct 
{
    u8 rgbBlue ;    //指定蓝色强度
    u8 rgbGreen ;	//指定绿色强度 
    u8 rgbRed ;	  	//指定红色强度 
    u8 rgbReserved ;//保留，设置为0 
}RGBQUAD ;
//位图信息头
typedef __packed struct
{ 
	BITMAPFILEHEADER bmfHeader;
	BITMAPINFOHEADER bmiHeader;  
	u32 RGB_MASK[3];			//调色板用于存放RGB掩码.
}BITMAPINFO; 
typedef RGBQUAD * LPRGBQUAD;//彩色表  

//图象数据压缩的类型
#define BI_RGB	 		0  //没有压缩.RGB 5,5,5.
#define BI_RLE8 		1  //每个象素8比特的RLE压缩编码，压缩格式由2字节组成(重复象素计数和颜色索引)；
#define BI_RLE4 		2  //每个象素4比特的RLE压缩编码，压缩格式由2字节组成
#define BI_BITFIELDS 	3  //每个象素的比特由指定的掩码决定。 （存储为RGB565格式位图时需要设置压缩类型为3） 
		       
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//BMP编解码函数
u8 stdbmp_decode(const u8 *filename);
u8 minibmp_decode(u8 *filename,u16 x,u16 y,u16 width,u16 height,u16 acolor,u8 mode);//尺寸小于240*320的bmp图片解码.
/* 打印位图文件头信息
*/
void printfBmpFileInfo(BITMAPFILEHEADER fileInfo);

/* 打印位图信息
*/
void printfBmpInfo(BITMAPINFOHEADER bmpInfo);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif








