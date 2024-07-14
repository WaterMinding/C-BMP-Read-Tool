#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

/* BMP文件头 */ 
typedef struct
{
	// 文件头大小
	int bfhSize; 
	 
	// 文件类型 
	short bfType; 
	
	// 文件大小
	int bfSize;
	
	// 保留字
	int bfReserves;
	
	// 位图数据偏移
	int bfOffBits;
	 
}BMP_FILE_HEADER;


/* BMP位图信息头 */
typedef struct
{
	// 信息头大小
	int biSize;
	
	// 图像宽度
	int biWidth;
	
	// 图像高度
	int biHeight; 
	
	// 平面数(总为1)
	short biPlanes;
	
	// 颜色位数
	short biBitCount;
	
	// 压缩类型
	int biCompression;
	
	// 位图数据大小
	int biSizeImages;
	
	// 水平分辨率
	int biXPelsPerMeter;
	
	// 垂直分辨率
	int biYPelsPerMeter;
	
	// 使用的颜色数
	int biClrUsed;
	
	// 重要的颜色数
	int biClrImportant;
	 
}BITMAP_INFORMATION;


/* 单像素点颜色 */
typedef struct
{
	unsigned char Blue;
	unsigned char Green;
	unsigned char Red;
	unsigned char Alpha;
	unsigned char Average;
	
}BMP_COLORS;

/* 完整图像结构体 */
typedef struct
{
	// 文件地址
	char* fileAddress;
	 
	// 文件头 
	BMP_FILE_HEADER bfh;
	
	// 位图信息头 
	BITMAP_INFORMATION bi;
	
	// 位图矩阵 
	BMP_COLORS** mat = NULL;
	
}BMP_IMAGE;


/* BMP文件读取函数 */
BMP_IMAGE BMP_Read(char* Address); 

/* 输出BMP灰度图及文件信息 */
void printBMPGray(BMP_IMAGE image);

/* 释放位图矩阵内存 */
void freeBMPMemory(BMP_IMAGE image);
