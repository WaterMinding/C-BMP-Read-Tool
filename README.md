## 简要介绍
- 本项目是一个由C语言编写的BMP图片读取工具，能够读取位深为8、24、32（没有验证过位深为32的图片）的BMP格式图片并保存为图片结构体对象。
## 整体架构
### 结构体
#### BMP文件头结构体
- **本结构体用于保存BMP文件的文件头信息**。
- BMP文件头是BMP文件开头的14个字节，保存了：
	- 代表BMP文件类型的编码bfType
		- 这个编码是 4d42(16进制)，如果某BMP文件不是这个编码，可能是文件已经损坏。
		- 显然这个编码占用 2字节。
	- 表示整个BMP文件大小的bfSize
		- 这个数值占用4字节。
	- 保留字 bfReserves
		- 保留字在一般BMP文件中为0。
		- BMP文件头中实际上有两个连续的保留字，各占2字节，这里统一起来占4字节。
	- 位图数据偏移 bfOffBits
		- 这个数值表示位图数据的首个字节在文件中的编号（编号从0开始）同时也能表示位图数据之前的所有信息的总大小。
		- 这个数值占4字节。
- 下方代码为BMP文件头结构体的C语言实现。
```C
/* BMP文件头 */ 
typedef struct
{
	// 文件头大小
	/*真实BMP文件的文件头并不包含文件头大小，因为这个值恒为14字节，变量bfhSize是本项               目为了方便后续代码的编写而添加在本结构体中的*/ 
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
```
#### BMP位图信息头结构体
- 在文件头以后就是位图信息头，保存了与位图信息相关的数据：
	- 表示信息头占用字节数的信息头大小biSize
		- 这个值一般为40。
		- 这个值本身占用4字节。
	- 表示位图宽度的biWidth
		- 这个值占用4字节
	- 表示位图高度 的biHeight
		- 若该值为正，说明位图信息在文件中是从位图矩阵的最后一行开始记录的，而后是倒数第二行，以此类推。若为负，则是正常的从第一行开始记录。但是无论哪种情况，行内的信息都是从左到右记录的。
		- 这个值占用4字节
	- 表示位图平面数的biPlanes
		- 一般该值为1。具体含义笔者不明。
		- 这个值占用2字节。
	- 表示位深的biBitCount
		- 这个数值表示编码一个像素点的颜色信息或灰度信息要用到的二进制位数，常见的BMP图像有 1位、4位、8位、16位、24位、32位。本项目只能读取其中的8位、24位和32位（32位的读取效果未经过实验验证）。
		- 这个值占用2字节。
	- 表示压缩类型的biCompression
		- 这个值一般为0表示不压缩，其他压缩方式笔者不清楚。
		- 这个值占用4字节。
	- 表示位图数据大小的biSizeImages
		- 这个值表示文件中的位图数据占用的字节数。
		- 这个值占用4字节。
	- 表示水平方向每米像素数的biXPelsPerMeter
		- 这个值占用4字节。
	- 表示竖直方向每米像素数的biYPelsPerMeter
		- 这个值占用4字节。
	- 表示位图使用的颜色数量的biClrUsed
		- 如果这个值为0，说明使用了 $2^{biBitCount}$  个颜色，即位深能编码的所有颜色。
		- 这个值占用4字节。
	- 表示位图使用的颜色中重要颜色数量的biClrImportant
		- 如果这个值为0，说明所有颜色都重要。
		- 这个值占用4字节。
- 下方代码为BMP位图信息头结构体的C语言实现
```C
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
```
#### 单像素点颜色结构体
- 在位深低于24位BMP文件中，在文件头和位图信息头之后，可能存在颜色索引表。索引表一般保存许多种BGR组合（没错，BMP的颜色顺序不是RGB而是BGR），这样一来，位图信息只需要保存索引表的索引序号，节省空间，需要显示某像素点时时，通过索引找到对应的BGR组合即可确定该像素的颜色。
- 在位深等于或高于24位的BMP文件中由于位深大，每个像素点直接编码BGR组合，不使用索引表。
	- 理由是：24位是三个字节，足够编码BGR信息，直接编码的方式允许表示 $2^{24}$ 种颜色。如果 用索引表表示颜色，要么罗列出所有 $2^{24}$ 种组合，这样会占用更大的存储空间；要么只罗列部分BGR组合，这样会使像素点能表示的颜色种类受到索引表规模的限制，导致图片颜色质量下降。两种方法都不如直接编码。
- 颜色索引表中的BGR组合一共占用4个字节，其中前三个字节分别表示Blue、Green、Red。最后一个字节是保留字必须为0，也有说法认为是透明度。本项目中作为透明度看待。
-  单像素点颜色结构体包括：
	- 表示蓝色灰度的Blue
	- 表示绿色灰度的Green
	- 表示红色灰度的Red
	- 表示保留字或透明度的Alpha
	- 表示蓝绿红三色灰度平均值的Average
		- Average并不是BMP文件索引表或像素点中保存的信息，是本项目添加的，方便获取平均灰度图的变量。
- 本项目使用单像素点颜色结构体数组保存颜色索引表， 也使用单像素点颜色结构体的二维数组保存位图信息矩阵。
```C
/* 单像素点颜色 */
typedef struct
{
	unsigned char Blue;
	unsigned char Green;
	unsigned char Red;
	unsigned char Alpha;
	// 下方变量不存在于真实BMP文件中，是本项目添加在结构体中的
	unsigned char Average;
	
}BMP_COLORS;
```
#### 完整图像结构体
- 完整图像结构体用来表示一个完全被读入内存的BMP图像，包括：
	- 记录 BMP文件地址（目录）的 fileAddress
	- 文件头结构体对象 bfh
	- 位图信息头结构体对象 bi
	- 位图矩阵指针 mat
		- 这个指针指向内存堆空间中开辟的一个二维数组，数组中保存图像的位图信息。
```C
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
```
### 函数
#### BMP文件读取函数
##### 基本信息
- 函数名：BMP_Read
- 参数：char* Address
- 返回：BMP_IMAGE对象
- 功能：读取BMP文件，并以BMP_IMAGE对象的形式保存在内存中。
##### 实现
- 声明后续要用的各种变量
- 其中bImage就是最终要返回的BMP_IMAGE对象
```C
	// 声明文件头变量
	BMP_FILE_HEADER bfh;
	bfh.bfhSize = 14;     //这里确定了文件头的大小
	
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
```

- 以二进制读入方式打开BMP文件，C语言基本操作，不多说。
```C
	// 以读取二进制方式打开图片文件 
	FILE* fp = fopen(Address,"rb");
	if(fp == NULL)
	{
		printf("图片打开失败");
		exit(-1); 
	}
```

- 读取BMP文件的文件头
- 逐个数据地使用fread()函数读取，由于这个函数的最小读取单位是字节，所以本项目不支持1位、4位位深的文件读取。 
- 读取之后对文件类型编码和保留字验证，如果前者不是 0x4d42 或后者不是 0 说明文件有问题。
```C
	// 读取文件头 
	if(feof(fp) == 0) //这里是怕文件损坏
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
```

- 读取BMP文件的信息头
- 仍然逐个使用fread()函数读取。
```C
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
```

- 提前判断高度数据的正负，顺便将负值转化为正值，方便后续循环操作。
```C
	// 判断位图高度数据的正负
	int heightFlag = (bi.biHeight<0);
	if(heightFlag == 1)
		bi.biHeight *= -1;
```

- 计算颜色索引表的长度，即求颜色索引表总共有多少项BGR组合
- 如果没有颜色索引表，则长度为0，即不需要创建颜色索引表。
- 先创建，后根据索引表长度循环读取，并不复杂，无需赘言。
```C
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
```

- 创建位图数据矩阵
- 首先通过malloc开辟长度为位图行数的指针数组matirx，其元素均为单像素颜色结构体的指针。
- 而后为指针数组matrix的每一个元素开辟长度为位图列数的单像素颜色结构体数组。
- 于是形成了二维数组。
```C
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
```

- 依照height的正负情况选择合适的方法读取位图信息
	- 如果为正，则将首先读到的行信息存入矩阵的最后一行，然后是倒数第二行，以此类推。
	- 如果为负，则将首先读到的行信息存入矩阵的第一行，然后是第二行，以此类推。
- 可以从代码看到上述两种读取方式，在最外层for循环的参数上，有所不同。
- 读取时，会按照8位、24位、32位的不同方式读取，因为位深不同，所以编码每个像素点的字节数量也就不同，所以读取方式不同。
- 值得注意的是，直接用Windows画图软件创建的BMP文件中，有时会在图像矩阵的每行末尾处加一些字节的0。这会导致实际宽度大于信息头中的宽度。但项目程序是按照信息头中记录的行列数开辟内存空间、建立二维数组，而后读取并保存位图信息的，第一行多出的字节会被当作第二行的开头读入，以此类推，全部数据都会受影响，位图最末尾的数据也不会被读取。
- 解决方案：图像在每行多出字节的同时，整体的位图信息大小也会增加，这个实际大小会被忠实地记录在信息头中，所以当位图信息大小大于图像宽高乘积时，就根据两者的差和图像的位深，计算出每行多余的字节数，读取文件时，在每一行都跳过这些字节，最终就能正确地读入位图信息。
```C
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
				for(k = 0;k < (bi.biSizeImages - bi.biHeight * bi.biWidth *(bi.biBitCount/8))/bi.biHeight;k++)
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
```

- 获取平均灰度
- 对位图矩阵中的每个像素点求Blue、Green、Red的平均值并保存在Average中。
```C
	// 获取灰度数据（BGR平均）
	for(i = 0;i < bi.biHeight;i++)
	{
		for(j = 0;j < bi.biWidth;j++)
			matrix[i][j].Average = int((matrix[i][j].Blue + matrix[i][j].Green + matrix[i][j].Red)/3);
	}
```

- 释放颜色索引表内存
- 上述代码中有三处malloc的使用，为了避免内存泄漏，这里需要释放内存。然而，后两处malloc开辟的空间都与位图矩阵相关，位图矩阵是该函数返回值的一部分，不能释放。故这里只需要释放颜色索引表内存（前提是有颜色索引表）。
```C
	// 释放颜色索引表内存 
	if (colorDictNum > 0)
		free(colorDict);
```

- 关闭文件
- 不必多说
```C
	// 关闭文件
	if(fclose(fp) == EOF)
	{
		printf("文件关闭失败");
		exit(-1);
	}
	fp = NULL;
```

- 将BMP文件的全部信息，打包到BMP_IMAGE对象。
- 最终返回BMP_IMAGE对象bImage。
```C
	// 打包到图像变量 
	bImage.fileAddress = Address;
	bImage.bfh = bfh;
	bImage.bi = bi;
	bImage.mat = matrix;

	return bImage;
```
##### 用法
- 函数 BMP_IMAGE BMP_Read(char* Address) 的用法很简单，只需要提供一个字符串形式的文件地址信息参数，再接收BMP_IMAGE类型的返回即可。
#### BMP灰度图输出函数
##### 基本信息
- 函数名：printBMPGray
- 参数：BMP_IMAGE image
- 返回：无返回 void
- 功能：输出BMP_IMAGE对象的文件头信息、位图信息头、位图矩阵信息。请不要提供无效的BMP_IMAGE对象（没有正确读入或保存位图矩阵信息的对象），这样做可能引起意料不到的危险后果。
##### 实现
- 本函数的实现是单纯输出，无需赘言。
```C
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
```
##### 用法
- 本函数只需要提供有效的BMP_IMAGE类型的对象，即可输出。请不要提供无效的BMP_IMAGE对象（没有正确读入或保存位图矩阵信息的对象），这样做可能引起意料不到的危险后果。
- 输出MINIST数据集第一张图片的效果如下所示：
```
.\测试图片\0_0.bmp

文件大小：1862
文件保留字：0
数据偏移量：1078
信息头大小：40
图片宽度：28
图片高度：28
图片平面数：1
图片位深：8
图片压缩方式：0
位图数据大小：784
水平方向分辨率：3780
垂直方向分辨率：3780
使用颜色数：256
重要颜色数：256

平均灰度图

00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 33 9f fd 9f 32 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 30 ee fc fc fc ed 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 36 e3 fd fc ef e9 fc 39 06 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 0a 3c e0 fc fd fc ca 54 fc fd 7a 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 a3 fc fc fc fd fc fc 60 bd fd a7 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 33 ee fd fd be 72 fd e4 2f 4f ff a8 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 30 ee fc fc b3 0c 4b 79 15 00 00 fd f3 32 00 00 00 00 00
00 00 00 00 00 00 00 00 26 a5 fd e9 d0 54 00 00 00 00 00 00 fd fc a5 00 00 00 00 00
00 00 00 00 00 00 00 07 b2 fc f0 47 13 1c 00 00 00 00 00 00 fd fc c3 00 00 00 00 00
00 00 00 00 00 00 00 39 fc fc 3f 00 00 00 00 00 00 00 00 00 fd fc c3 00 00 00 00 00
00 00 00 00 00 00 00 c6 fd be 00 00 00 00 00 00 00 00 00 00 ff fd c4 00 00 00 00 00
00 00 00 00 00 00 4c f6 fc 70 00 00 00 00 00 00 00 00 00 00 fd fc 94 00 00 00 00 00
00 00 00 00 00 00 55 fc e6 19 00 00 00 00 00 00 00 00 07 87 fd ba 0c 00 00 00 00 00
00 00 00 00 00 00 55 fc df 00 00 00 00 00 00 00 00 07 83 fc e1 47 00 00 00 00 00 00
00 00 00 00 00 00 55 fc 91 00 00 00 00 00 00 00 30 a5 fc ad 00 00 00 00 00 00 00 00
00 00 00 00 00 00 56 fd e1 00 00 00 00 00 00 72 ee fd a2 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 55 fc f9 92 30 1d 55 b2 e1 fd df a7 38 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 55 fc fc fc e5 d7 fc fc fc c4 82 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 1c c7 fc fc fd fc fc e9 91 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 19 80 fc fd fc 8d 25 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

红色灰度图

00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 33 9f fd 9f 32 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 30 ee fc fc fc ed 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 36 e3 fd fc ef e9 fc 39 06 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 0a 3c e0 fc fd fc ca 54 fc fd 7a 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 a3 fc fc fc fd fc fc 60 bd fd a7 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 33 ee fd fd be 72 fd e4 2f 4f ff a8 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 30 ee fc fc b3 0c 4b 79 15 00 00 fd f3 32 00 00 00 00 00
00 00 00 00 00 00 00 00 26 a5 fd e9 d0 54 00 00 00 00 00 00 fd fc a5 00 00 00 00 00
00 00 00 00 00 00 00 07 b2 fc f0 47 13 1c 00 00 00 00 00 00 fd fc c3 00 00 00 00 00
00 00 00 00 00 00 00 39 fc fc 3f 00 00 00 00 00 00 00 00 00 fd fc c3 00 00 00 00 00
00 00 00 00 00 00 00 c6 fd be 00 00 00 00 00 00 00 00 00 00 ff fd c4 00 00 00 00 00
00 00 00 00 00 00 4c f6 fc 70 00 00 00 00 00 00 00 00 00 00 fd fc 94 00 00 00 00 00
00 00 00 00 00 00 55 fc e6 19 00 00 00 00 00 00 00 00 07 87 fd ba 0c 00 00 00 00 00
00 00 00 00 00 00 55 fc df 00 00 00 00 00 00 00 00 07 83 fc e1 47 00 00 00 00 00 00
00 00 00 00 00 00 55 fc 91 00 00 00 00 00 00 00 30 a5 fc ad 00 00 00 00 00 00 00 00
00 00 00 00 00 00 56 fd e1 00 00 00 00 00 00 72 ee fd a2 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 55 fc f9 92 30 1d 55 b2 e1 fd df a7 38 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 55 fc fc fc e5 d7 fc fc fc c4 82 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 1c c7 fc fc fd fc fc e9 91 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 19 80 fc fd fc 8d 25 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

绿色灰度图

00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 33 9f fd 9f 32 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 30 ee fc fc fc ed 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 36 e3 fd fc ef e9 fc 39 06 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 0a 3c e0 fc fd fc ca 54 fc fd 7a 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 a3 fc fc fc fd fc fc 60 bd fd a7 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 33 ee fd fd be 72 fd e4 2f 4f ff a8 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 30 ee fc fc b3 0c 4b 79 15 00 00 fd f3 32 00 00 00 00 00
00 00 00 00 00 00 00 00 26 a5 fd e9 d0 54 00 00 00 00 00 00 fd fc a5 00 00 00 00 00
00 00 00 00 00 00 00 07 b2 fc f0 47 13 1c 00 00 00 00 00 00 fd fc c3 00 00 00 00 00
00 00 00 00 00 00 00 39 fc fc 3f 00 00 00 00 00 00 00 00 00 fd fc c3 00 00 00 00 00
00 00 00 00 00 00 00 c6 fd be 00 00 00 00 00 00 00 00 00 00 ff fd c4 00 00 00 00 00
00 00 00 00 00 00 4c f6 fc 70 00 00 00 00 00 00 00 00 00 00 fd fc 94 00 00 00 00 00
00 00 00 00 00 00 55 fc e6 19 00 00 00 00 00 00 00 00 07 87 fd ba 0c 00 00 00 00 00
00 00 00 00 00 00 55 fc df 00 00 00 00 00 00 00 00 07 83 fc e1 47 00 00 00 00 00 00
00 00 00 00 00 00 55 fc 91 00 00 00 00 00 00 00 30 a5 fc ad 00 00 00 00 00 00 00 00
00 00 00 00 00 00 56 fd e1 00 00 00 00 00 00 72 ee fd a2 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 55 fc f9 92 30 1d 55 b2 e1 fd df a7 38 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 55 fc fc fc e5 d7 fc fc fc c4 82 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 1c c7 fc fc fd fc fc e9 91 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 19 80 fc fd fc 8d 25 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

蓝色灰度图

00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 33 9f fd 9f 32 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 30 ee fc fc fc ed 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 36 e3 fd fc ef e9 fc 39 06 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 0a 3c e0 fc fd fc ca 54 fc fd 7a 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 a3 fc fc fc fd fc fc 60 bd fd a7 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 33 ee fd fd be 72 fd e4 2f 4f ff a8 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 30 ee fc fc b3 0c 4b 79 15 00 00 fd f3 32 00 00 00 00 00
00 00 00 00 00 00 00 00 26 a5 fd e9 d0 54 00 00 00 00 00 00 fd fc a5 00 00 00 00 00
00 00 00 00 00 00 00 07 b2 fc f0 47 13 1c 00 00 00 00 00 00 fd fc c3 00 00 00 00 00
00 00 00 00 00 00 00 39 fc fc 3f 00 00 00 00 00 00 00 00 00 fd fc c3 00 00 00 00 00
00 00 00 00 00 00 00 c6 fd be 00 00 00 00 00 00 00 00 00 00 ff fd c4 00 00 00 00 00
00 00 00 00 00 00 4c f6 fc 70 00 00 00 00 00 00 00 00 00 00 fd fc 94 00 00 00 00 00
00 00 00 00 00 00 55 fc e6 19 00 00 00 00 00 00 00 00 07 87 fd ba 0c 00 00 00 00 00
00 00 00 00 00 00 55 fc df 00 00 00 00 00 00 00 00 07 83 fc e1 47 00 00 00 00 00 00
00 00 00 00 00 00 55 fc 91 00 00 00 00 00 00 00 30 a5 fc ad 00 00 00 00 00 00 00 00
00 00 00 00 00 00 56 fd e1 00 00 00 00 00 00 72 ee fd a2 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 55 fc f9 92 30 1d 55 b2 e1 fd df a7 38 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 55 fc fc fc e5 d7 fc fc fc c4 82 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 1c c7 fc fc fd fc fc e9 91 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 19 80 fc fd fc 8d 25 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

--------------------------------
Process exited after 0.4431 seconds with return value 0
请按任意键继续. . .
```

#### 位图矩阵内存释放函数
##### 基本信息
- 函数名：freeBMPMemory
- 参数：BMP_IMAGE image
- 返回：无返回 void
- 功能：提供有效的BMP_IMAGE对象，即可释放该对象中位图矩阵的内存。请不要提供无效的BMP_IMAGE对象（没有正确读入或保存位图矩阵信息的对象），这样做可能引起意料不到的危险后果。
##### 实现
- 首先循环释放矩阵每一行的空间。
- 而后释放矩阵行指针空间。
- 最终将指针置空避免野指针出现。
```C
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
```
##### 用法
- 提供有效的BMP_IMAGE对象，即可释放该对象中位图矩阵的内存。请不要提供无效的BMP_IMAGE对象（没有正确读入或保存位图矩阵信息的对象），这样做可能引起意料不到的危险后果。
## 参考文献
- [BMP文件格式解析_bmp文件格式详解-CSDN博客](https://blog.csdn.net/XYK0318/article/details/105123197)
- [BMP图片格式分析（超详细）-CSDN博客](https://blog.csdn.net/weixin_50964512/article/details/128646165)