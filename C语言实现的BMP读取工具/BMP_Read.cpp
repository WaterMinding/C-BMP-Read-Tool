# include "BMP_Read.h"


/* BMP文件读取函数 */
BMP_IMAGE BMP_Read(char* Address)
{
	// 声明文件头变量
	BMP_FILE_HEADER bfh;
	bfh.bfhSize = 14;
	
	// 声明信息头变量
	BITMAP_INFORMATION bi;
	
	// 声明图像变量
	BMP_IMAGE bImage; 
	
	// 声明循环控制变量
	int i = 0;
	int j = 0;
	int k = 0;
	
	// 声明缓存变量
	int temp = 0;
	int empty = 0; 
	
	// 声明颜色索引表
	BMP_COLORS* colorDict; 
	
	// 以读取二进制方式打开图片文件 
	FILE* fp = fopen(Address,"rb");
	if(fp == NULL)
	{
		printf("图片打开失败");
		exit(-1); 
	}
	
	// 读取文件头 
	if(feof(fp) == 0)
	{
		// 读取文件类型
		fread(&(bfh.bfType),1,sizeof(bfh.bfType),fp);
		
		// 读取文件大小
		fread(&(bfh.bfSize),sizeof(bfh.bfSize),1,fp);
		
		// 读取文件保留字
		fread(&(bfh.bfReserves),sizeof(bfh.bfReserves),1,fp);
		
		// 读取数据偏移量
		fread(&(bfh.bfOffBits),sizeof(bfh.bfOffBits),1,fp);
	}
	else
	{
		printf("文件不完整");
		exit(-1);		
	}
	
	// 验证文件头 
	if(bfh.bfType != 0x4d42 || bfh.bfReserves != 0)
	{
		printf("文件损坏，或文件并非BMP格式！\n");
		exit(-1);
	}
	
	// 读取信息头
	if(feof(fp) == 0)
	{
		// 读取信息头长度
		fread(&(bi.biSize),1,sizeof(bi.biSize),fp);

		// 读取图片宽度
		fread(&(bi.biWidth),1,sizeof(bi.biWidth),fp);

		// 读取图片高度
		fread(&(bi.biHeight),1,sizeof(bi.biHeight),fp); 

		// 读取图片平面数
		fread(&(bi.biPlanes),1,sizeof(bi.biPlanes),fp);

		// 读取图片颜色位数（深度）
		fread(&(bi.biBitCount),1,sizeof(bi.biBitCount),fp);

		// 读取图片压缩方式
		fread(&(bi.biCompression),1,sizeof(bi.biCompression),fp);

		// 读取位图数据长度
		fread(&(bi.biSizeImages),1,sizeof(bi.biSizeImages),fp);

		// 读取水平方向分辨率
		fread(&(bi.biXPelsPerMeter),1,sizeof(bi.biXPelsPerMeter),fp);

		// 读取垂直方向分辨率
		fread(&(bi.biYPelsPerMeter),1,sizeof(bi.biYPelsPerMeter),fp);

		// 读取使用颜色数
		fread(&(bi.biClrUsed),1,sizeof(bi.biClrUsed),fp);

		// 读取重要颜色数
		fread(&(bi.biClrImportant),1,sizeof(bi.biClrImportant),fp);

	}
	else
	{
		printf("文件不完整");
		exit(-1);		
	}
	
	// 判断位图高度数据的正负
	int heightFlag = (bi.biHeight<0);
	if(heightFlag == 1)
		bi.biHeight *= -1;
	 
	
	// 计算颜色索引表的项数 
	int colorDictNum = (bfh.bfOffBits - (bfh.bfhSize + bi.biSize))/4;
	
	// 如果文件有颜色索引表则创建颜色索引表 
	if(colorDictNum != 0)
	{
		// 创建颜色索引表数组
		colorDict = (BMP_COLORS*)malloc(colorDictNum * sizeof(BMP_COLORS));
		if(colorDict == NULL)
		{
			printf("颜色索引表数组：内存空间开辟失败");
			exit(-1);
		}
		
		// 读取索引表
		for(i = 0;i<colorDictNum;i++)
		{
			fread(&colorDict[i].Blue,1,1,fp);
			fread(&colorDict[i].Green,1,1,fp);
			fread(&colorDict[i].Red,1,1,fp);
			fread(&colorDict[i].Alpha,1,1,fp);
		}
	}
	
	// 创建位图数据矩阵
	BMP_COLORS** matrix = (BMP_COLORS**)malloc(bi.biHeight * sizeof(BMP_COLORS*));
	if(matrix == NULL)
	{
		printf("位图数据数组：内存空间开辟失败");
		exit(-1);
	}
	for(i = 0;i < bi.biHeight;i++)
	{
		matrix[i] = (BMP_COLORS*)malloc(bi.biWidth * sizeof(BMP_COLORS));
		if(matrix[i] == NULL)
		{
			printf("位图数据数组：内存空间开辟失败");
			exit(-1);
		}
	}
	
	// 按照height的正负选择合适的位图存储方式 
	if(heightFlag == 0)
	{
		// 读取位图数据并解压存储 
		for(i = bi.biHeight - 1;i >= 0;i--)
		{
			for(j = 0;j < bi.biWidth;j++)
			{
				// 区分位深，按字节地读取单像素的颜色信息 
				// 8位深（含有索引表） 
				if(bi.biBitCount == 8)
				{
					fread(&temp,1,1,fp);
					matrix[i][j] = colorDict[temp];
				}
				// 24位深（不含索引表）
				else if(bi.biBitCount ==24)
				{
					fread(&temp,1,1,fp);
					matrix[i][j].Blue = temp;
					fread(&temp,1,1,fp);
					matrix[i][j].Green = temp;
					fread(&temp,1,1,fp);
					matrix[i][j].Red = temp;
				}
				// 32位深（不含索引表）
				else if(bi.biBitCount ==32)
				{
					fread(&temp,1,1,fp);
					matrix[i][j].Blue = temp;
					fread(&temp,1,1,fp);
					matrix[i][j].Green = temp;
					fread(&temp,1,1,fp);
					matrix[i][j].Red = temp;
					fread(&temp,1,1,fp);
					matrix[i][j].Alpha = temp;
				}
			}
			
			// 如果实际位图数据大小超过长宽乘积，则每行跳过部分字节。
			// 这是因为画图软件创建的BMP文件有时会在每行末尾单独加一定字节的0值
			// 为避免此类现象对图像读取的干扰，这里跳过行末尾字节 
			if(bi.biSizeImages > bi.biHeight * bi.biWidth)
			{
				for(k = 0;k < (bi.biSizeImages - bi.biHeight * bi.biWidth * (bi.biBitCount/8))/bi.biHeight;k++)
					fread(&empty,1,1,fp);
			} 
		}
	}
	else
	{
		// 读取位图数据并解压存储 
		for(i = 0;i < bi.biHeight ;i++)
		{
			for(j = 0;j < bi.biWidth;j++)
			{
				// 区分位深，按字节地读取单像素的颜色信息 
				// 8位深（含有索引表）
				if(bi.biBitCount == 8)
				{
					fread(&temp,1,1,fp);
					matrix[i][j] = colorDict[temp];
				}
				// 24位深（不含索引表）
				else if(bi.biBitCount ==24)
				{
					fread(&temp,1,1,fp);
					matrix[i][j].Blue = temp;
					fread(&temp,1,1,fp);
					matrix[i][j].Green = temp;
					fread(&temp,1,1,fp);
					matrix[i][j].Red = temp;
				}
				// 32位深（不含索引表）
				else if(bi.biBitCount ==32)
				{
					fread(&temp,1,1,fp);
					matrix[i][j].Blue = temp;
					fread(&temp,1,1,fp);
					matrix[i][j].Green = temp;
					fread(&temp,1,1,fp);
					matrix[i][j].Red = temp;
					fread(&temp,1,1,fp);
					matrix[i][j].Alpha = temp;
				}
			}
			
			// 如果实际位图数据大小超过长宽乘积，则每行跳过部分字节。
			// 这是因为画图软件创建的BMP文件有时会在每行末尾单独加一定字节的0值
			// 为避免此类现象对图像读取的干扰，这里跳过行末尾字节 
			if(bi.biSizeImages > bi.biHeight * bi.biWidth)
			{
				for(k = 0;k < (bi.biSizeImages - bi.biHeight * bi.biWidth * (bi.biBitCount/8))/bi.biHeight;k++)
					fread(&empty,1,1,fp);
			}
		}	
	}
	
	// 获取灰度数据（BGR平均）
	for(i = 0;i < bi.biHeight;i++)
	{
		for(j = 0;j < bi.biWidth;j++)
			matrix[i][j].Average = int((matrix[i][j].Blue + matrix[i][j].Green + matrix[i][j].Red)/3);
	}
	
	// 释放颜色索引表内存 
	if (colorDictNum > 0)
		free(colorDict);
	
	// 关闭文件
	if(fclose(fp) == EOF)
	{
		printf("文件关闭失败");
		exit(-1);
	}
	fp = NULL;
	
	// 打包到图像变量 
	bImage.fileAddress = Address;
	bImage.bfh = bfh;
	bImage.bi = bi;
	bImage.mat = matrix;
	
	return bImage;
}

/* 输出BMP灰度图及文件信息 */
void printBMPGray(BMP_IMAGE image)
{
	// 设置循环控制变量 
	int i = 0,j = 0; 
	
	// 输出文件信息
	printf("---------------------------------------------------------------------------------------------------------------------\n");
	printf("%s\n\n",image.fileAddress);
	printf("文件大小：%d\n",image.bfh.bfSize);
	printf("文件保留字：%d\n",image.bfh.bfReserves);
	printf("数据偏移量：%d\n",image.bfh.bfOffBits);
	printf("信息头大小：%d\n",image.bi.biSize);
	printf("图片宽度：%d\n",image.bi.biWidth);
	printf("图片高度：%d\n",image.bi.biHeight);
	printf("图片平面数：%d\n",image.bi.biPlanes);
	printf("图片位深：%d\n",image.bi.biBitCount);
	printf("图片压缩方式：%d\n",image.bi.biCompression);
	printf("位图数据大小：%d\n",image.bi.biSizeImages);
	printf("水平方向分辨率：%d\n",image.bi.biXPelsPerMeter);
	printf("垂直方向分辨率：%d\n",image.bi.biYPelsPerMeter);
	printf("使用颜色数：%d\n",image.bi.biClrUsed);
	printf("重要颜色数：%d\n",image.bi.biClrImportant);
	
	// 输出平均灰度图
	printf("\n平均灰度图\n\n");
	for(i = 0;i < image.bi.biHeight;i++)
	{
		for(j = 0;j < image.bi.biWidth;j++)
		{
			if(image.mat[i][j].Average <= 15)
				printf("%x",0);
			printf("%x ",image.mat[i][j].Average);
		}			
		printf("\n");
	}
	
	// 输出红色灰度图
	printf("\n红色灰度图\n\n");
	for(i = 0;i < image.bi.biHeight;i++)
	{
		for(j = 0;j < image.bi.biWidth;j++)
		{
			if(image.mat[i][j].Red <= 15)
				printf("%x",0);
			printf("%x ",image.mat[i][j].Red);
		}			
		printf("\n");
	}
	
	// 输出绿色灰度图
	printf("\n绿色灰度图\n\n");
	for(i = 0;i < image.bi.biHeight;i++)
	{
		for(j = 0;j < image.bi.biWidth;j++)
		{
			if(image.mat[i][j].Green <= 15)
				printf("%x",0);
			printf("%x ",image.mat[i][j].Green);
		}			
		printf("\n");
	}
	
	// 输出蓝色灰度图
	printf("\n蓝色灰度图\n\n");
	for(i = 0;i < image.bi.biHeight;i++)
	{
		for(j = 0;j < image.bi.biWidth;j++)
		{
			if(image.mat[i][j].Blue <= 15)
				printf("%x",0);
			printf("%x ",image.mat[i][j].Blue);
		}			
		printf("\n");
	} 
	
}

/* 内存释放函数 */
void freeBMPMemory(BMP_IMAGE image)
{
	// 设置循环控制变量
	int i = 0;
	
	// 释放内存 
	for(i = 0;i < image.bi.biHeight;i++)
		free(image.mat[i]);
	free(image.mat);
	image.mat = NULL;
}

