#ifndef __IMAGEPROCESS__
#define __IMAGEPROCESS__
#include "ff.h"
#include "stm32f10x.h"

/* 图像灰度化
 * 思路：定义两个文件指针，
 * 第一个文件指针用于读取原图像中的像素值，
 * 第二个用来指向待写入的文件
 * RGB565格式掩码
			bmp.RGB_MASK[0] = 0X00F800;
			bmp.RGB_MASK[1] = 0X0007E0;
			bmp.RGB_MASK[2] = 0X00001F;
 */
void Graying(const TCHAR* src, const TCHAR* dist);

//ostu二值化算法
/*
 * 思路：定义两个文件指针，
 * 第一个文件指针用于读取原图像中的像素值，
 * 第二个用来指向待写入的文件
 * f3:数据文件
 */
void Ostu(const TCHAR* src, const TCHAR* dist);

/* BP神经网络识别数字
 * 思路：已知权值矩阵w、v，以及阀值矩阵b1、b2
 * 需要使用四个文件：
 *	第1个文件指针指向权值矩阵w
 * 	第2个用来指针指向权值矩阵v
 * 	第3个用来指针指向阀值矩阵b1
 * 	第4个用来指针指向阀值矩阵b2
 * 另外src为分割后的单个待识别图片的路径
 */
u8 BP_Recongnization(const TCHAR* src);



#endif