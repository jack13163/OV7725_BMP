#include "ff.h"
#include "bmp.h"
#include "math.h"
#include "stdio.h"
#include "imageprocess.h"

//常用颜色定义
#define BLACK 0x0000
#define WHITE 0xFFFF

//文件指针（一定要定义在外部，否则可能出现堆栈溢出后果，程序死掉）
FIL f1, f2, f3, f4, f5, f6, f7, f8;
BITMAPINFO bmp;

/* 图像灰度化
 * 思路：定义两个文件指针，
 * 第一个文件指针用于读取原图像中的像素值，
 * 第二个用来指向待写入的文件
 * RGB565格式掩码
			bmp.RGB_MASK[0] = 0X00F800;
			bmp.RGB_MASK[1] = 0X0007E0;
			bmp.RGB_MASK[2] = 0X00001F;
 */
void Graying(const TCHAR* src, const TCHAR* dist)
{
	UINT num = 0;
	u16 i,j;
	u16 color;
	u8 r,g,b,ret;
	u16 width, height;
	u8 t = 0;
	u8 res1, res2;
	
	//打开文件
	do
	{
		res1 = f_open(&f1, src, FA_READ | FA_WRITE);
		res2 = f_open(&f2, dist, FA_READ | FA_WRITE);
	}while((FR_OK != res1) || (FR_OK != res2));
	
	f_read(&f1, &bmp, sizeof(bmp), &num);
	f_write(&f2, &bmp, sizeof(bmp), &num);
		
	height = bmp.bmiHeader.biHeight;
	width = bmp.bmiHeader.biWidth;
	
	for(i = 0; i < width; i++)
	{
		for(j = 0; j < height; j++)
		{
			f_read(&f1, &color, sizeof(color), &num);
			r = (color & 0XF800) >> 11;
			g = (color & 0X07E0) >> 6;
			b = color & 0X001F;
			ret = r * 0.299 + g * 0.587 + b * 0.114;
			color = (ret << 11) + (ret << 6) + ret;
			f_write(&f2, &color, sizeof(color), &num);
		}
	}
	//关闭文件
	f_close(&f2);
	f_close(&f1);
}

//ostu二值化算法
/*
 * 思路：定义两个文件指针，
 * 第一个文件指针用于读取原图像中的像素值，
 * 第二个用来指向待写入的文件
 * f3:数据文件
 */
void Ostu(const TCHAR* src, const TCHAR* dist)
{
	UINT num = 0;
	u32 i,j;
	u16 color;
	u8 gray;
	u16 width, height;
	u8 t = 0;
	//文件打开标志
	u8 res1, res2, res3; 
	u32 dcount = 0;//数据计数
	int Th = 0;//阀值变量
	//定义背景和目标像素数目变量N1,N2，灰度变量U1,U2
	int N1 = 0, N2 = 0;
	//灰度和变量Sum1,Sum2
  int Sum1 = 0, Sum2 = 0;
	//图像整体平均灰度变量U
  double U1 = 0, U2 = 0;
	//方差变量gr，对比阈值变量TT
  long long g = 0, TT = 0;
	
	//打开文件
	do
	{
		res1 = f_open(&f1, src, FA_READ | FA_WRITE);
		res2 = f_open(&f2, dist, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
		res3 = f_open(&f3, "datafile.dat", FA_CREATE_ALWAYS | FA_READ | FA_WRITE);//每次打开都新建,用于临时存放灰度数据（一个字节代表一个灰度值）
	}while((FR_OK != res1) || (FR_OK != res2) || (FR_OK != res3));
	
	f_read(&f1, &bmp, sizeof(bmp), &num);
	f_write(&f2, &bmp, sizeof(bmp), &num);
	
	height = bmp.bmiHeader.biHeight;
	width = bmp.bmiHeader.biWidth;
	dcount = height * width;
	
	//读取灰度值，并写入数据文件
	for(i = 0; i < width; i++)
	{
		for(j = 0; j < height; j++)
		{
			f_read(&f1, &color, sizeof(color), &num);
			gray = color & 0X001F;
			f_write(&f3, &gray, sizeof(gray), &num);
		}
	}
	
	//寻找最大类间方差
	for (i = 0; i <= 255; i++)
  {
		//统计两个类的元素个数和像素灰度和
		for (j = 0; j < dcount; j++)
		{
			//获取第j个字节的数据
			f_lseek(&f3, j);
			f_read(&f3, &gray, sizeof(gray), &num);
			if(gray > i)
			{
				N2++;
				Sum2 += gray;
			}
			else
			{
				N1++;
				Sum1 += gray;
			}
		}
		U1 = (N1 == 0 ? 0.0 : (Sum1 / N1));//类1像素均值
		U2 = (N2 == 0 ? 0.0 : (Sum2 / N2));
		g = N1 * N2 * (U1 - U2) * (U1 - U2);//计算两个分类之间的方差
		if (g > TT)//保存最大的方差
		{
			TT = g;//保存最大的方差
			Th = i;//以此时的灰度值为阀值
		}
		N1 = 0; N2 = 0;
		Sum1 = 0; Sum2 = 0; U1 = 0.0; U2 = 0.0; g = 0.0;
	}
	
	f_lseek(&f3, 0);
	//读取灰度值，并写入数据文件
	for(i = 0; i < width; i++)
	{
		for(j = 0; j < height; j++)
		{
			f_read(&f3, &gray, sizeof(gray), &num);
			if(gray < Th)
			{
				color = BLACK;
			}
			else
			{
				color = WHITE;
			}
			f_write(&f2, &color, sizeof(color), &num);
		}
	}
	
	//关闭文件
	f_close(&f2);
	f_close(&f1);
	f_close(&f3);
}

/* 从txt文件中读取一个浮点型数据
 * fp: Pointer to the blank file object
 */
void readDouble(FIL *fp, double* ret)
{
	u8 s[2];
	UINT rc;
	char p[30];
	int i = 0, j = 0;
	double tmp1 = 0;
	double tmp2 = 0;
	double sum1 = 0;
	double sum2 = 0;
	int count = 0;
	
	//初始化
	for(i = 0; i < 30; i++)
	{
		p[i] = ' ';
	}
	
	i = 0;
	//1.读取所有的数字
	do
	{
		f_read(fp, s, 1, &rc);
		if(rc != 1) //EOF或出错
		{
			break;
		}
		
		//换行符处理
		if(s[0] == '\n' || s[0] == '\r')
		{
			continue;
		}
		
		p[i] = s[0];
		if (p[i] == ' ')
		{
			break;
		}
		i++;
	}while(1);
	i = 0;
	//2.将字符转换为浮点数
	if(p[0] == '-')
	{
		i = 1;
		//1.获取整数部分，直到遇到.结束循环
		while(p[i] != '.')
		{
			tmp1 = p[i] - '0';
			sum1 *= 10;//sum向前进位
			sum1 += tmp1;//加上当前位
			i++;
		}
		i++;//跳过.
		//2.获取小数部分
		while(p[i] != ' ')
		{
			count++;
			tmp2 = p[i] - '0';
			j = 0;
			while(j < count)
			{
				tmp2 *= 0.1;
				j++;
			}
			sum2 += tmp2;
			i++;
		}
		//3.合成浮点数
		*ret = -(sum1 + sum2);
	}
	else
	{
		//1.获取整数部分，直到遇到.结束循环
		while(p[i] != '.')
		{
			tmp1 = p[i] - '0';
			sum1 *= 10;//sum向前进位
			sum1 += tmp1;//加上当前位
			i++;
		}
		i++;//跳过.
		//2.获取小数部分
		while(p[i] != ' ')
		{
			count++;
			tmp2 = p[i] - '0';
			j = 0;
			while(j < count)
			{
				tmp2 *= 0.1;
				j++;
			}
			sum2 += tmp2;
			i++;
		}
		//3.合成浮点数
		*ret = sum1 + sum2;
	}
}

/* 从txt文件中随机读取一个浮点型数据
 * n: 第几个浮点型数据
 */
void readDoubleRandom(FIL *fp, double* ret, int n)
{
	u8 s[2];
	UINT rc;
	int count = 0;
	f_lseek(fp, 0);//重定位到文件开头
	while(count < n)
	{
		f_read(fp, s, 1, &rc);
		if(rc != 1) //EOF或出错
		{
			break;
		}
		if (s[0] == ' ')
		{
			count++;
		}
	}
	readDouble(fp, ret);
}

/* BP神经网络识别数字
 * 思路：已知权值矩阵w、v，以及阀值矩阵b1、b2
 * 需要使用四个文件：
 *	第1个文件指针指向权值矩阵w
 * 	第2个用来指针指向权值矩阵v
 * 	第3个用来指针指向阀值矩阵b1
 * 	第4个用来指针指向阀值矩阵b2
 * 另外src为分割后的单个待识别图片的路径
 */
u8 BP_Recongnization(const TCHAR* src)
{
	//文件打开标志
	u8 res1, res2, res3, res4, res5, res6, res7, res8;
	u8 color[4];
	u8 gray;
	double pMax;//最大的灰度值
	double tmp;
	UINT num = 0;
	double d;
	u32 height, width, i, j;
	int hideNum, inNum, outNum, t;
	double t1, t2, t3, t4, t5;
	
	double max;
	int maxi = 0;
	
	//打开文件
	do
	{
		res1 = f_open(&f1, "0:BP/w.txt", FA_READ);
		res2 = f_open(&f2, "0:BP/v.txt", FA_READ);
		res3 = f_open(&f3, "0:BP/b1.txt", FA_READ);
		res4 = f_open(&f4, "0:BP/b2.txt", FA_READ);
		res5 = f_open(&f5, "0:BP/x1.dat", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
		res6 = f_open(&f6, "0:BP/x2.dat", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
		res7 = f_open(&f7, src, FA_READ);
		res8 = f_open(&f8, "0:BP/x.dat", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
	}while((FR_OK != res1) || (FR_OK != res2) || (FR_OK != res3) || (FR_OK != res4) || (FR_OK != res5) || (FR_OK != res6) || (FR_OK != res7) || (FR_OK != res8));
	//1.数据归一化
	f_read(&f7, &bmp, sizeof(bmp), &num);
	
	height = bmp.bmiHeader.biHeight;
	width = bmp.bmiHeader.biWidth;
	inNum = height * width;
	outNum = 10;
	t = sqrt(0.43*inNum*outNum + 0.12*outNum*outNum + 2.54*inNum + 0.77*outNum + 0.35) + 0.51;//由输入节点数和输出节点数计算得到
	hideNum = (int)(t);
	hideNum = (t - hideNum * 1.0) > 0.5 ? hideNum +1 : hideNum;
	
	//打印位图信息
	printfBmpFileInfo(bmp.bmfHeader);
	printfBmpInfo(bmp.bmiHeader);
	
	//定位到位图数据区域
	f_lseek(&f7, bmp.bmfHeader.bfOffBits); 
	//查找最大值
	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			f_read(&f7, &color, sizeof(color), &num);//输入
			gray = color[0];
			if(gray > pMax)
			{
				pMax = gray;
			}
		}
	}
	
	printf("\r\n");
	//定位到位图数据区域
	f_lseek(&f7, bmp.bmfHeader.bfOffBits);
	//利用最大值归一化，同时保存到一个临时的文件中
	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			f_read(&f7, &color, sizeof(color), &num);//输入
			gray = color[0];//r
			
			tmp = gray / pMax;
			//printf("%f ", tmp);
			f_write(&f8, &tmp, sizeof(tmp), &num);//x
		}
	}
	
	//2.计算隐藏层的输入输出
	for(i = 0; i < hideNum; i++)
	{
		tmp = 0.0;
		f_lseek(&f8, 0);
		for(j = 0; j < inNum; j++)
		{
			//读取一个双精度浮点型数据
			readDouble(&f1, &t1);//w
			f_read(&f8, &t2, sizeof(t2), &num);//x
			
			tmp += t1 * t2;
			
		}
		readDouble(&f3, &t3);//b1
		
		t5 = 1.0 / (1.0 + exp(-t3 - tmp));
		f_write(&f5, &t5, sizeof(t5), &num);//x1
	}
	
	f_lseek(&f5, 0);
	//3.计算输出层的输入输出
	for(i = 0; i < outNum; i++)
	{
		tmp = 0.0;
		f_lseek(&f5, 0);
		for(j = 0; j < hideNum; j++)
		{
			//读取一个双精度浮点型数据
			readDouble(&f2, &t1);//v
			f_read(&f5, &t2, sizeof(t2), &num);//x1
			
			tmp += t1 * t2;
		}
		readDouble(&f4, &t3);//b2
		t5 = 1.0 / (1.0 + exp(-t3 - tmp));
		f_write(&f6, &t5, sizeof(t5), &num);//x2
	}
	
	f_lseek(&f6, 0);
	//4.求输出层最大值所在的节点
	f_read(&f6, &max, sizeof(max), &num);//x2
	for(i = 1; i < outNum; i++)
	{
		f_read(&f6, &tmp, sizeof(tmp), &num);//x2
		if(tmp > max)
		{
			max = tmp;
			maxi = i;
		}
	}
	printf("识别结果:%d\r\n\r\n", maxi);
	//关闭文件
	f_close(&f1);
	f_close(&f2);
	f_close(&f3);
	f_close(&f4);
	f_close(&f5);
	f_close(&f6);
	f_close(&f7);
	f_close(&f8);
	return maxi;
}