# include "BMP_Read.h"


/* BMP�ļ���ȡ���� */
BMP_IMAGE BMP_Read(char* Address)
{
	// �����ļ�ͷ����
	BMP_FILE_HEADER bfh;
	bfh.bfhSize = 14;
	
	// ������Ϣͷ����
	BITMAP_INFORMATION bi;
	
	// ����ͼ�����
	BMP_IMAGE bImage; 
	
	// ����ѭ�����Ʊ���
	int i = 0;
	int j = 0;
	int k = 0;
	
	// �����������
	int temp = 0;
	int empty = 0; 
	
	// ������ɫ������
	BMP_COLORS* colorDict; 
	
	// �Զ�ȡ�����Ʒ�ʽ��ͼƬ�ļ� 
	FILE* fp = fopen(Address,"rb");
	if(fp == NULL)
	{
		printf("ͼƬ��ʧ��");
		exit(-1); 
	}
	
	// ��ȡ�ļ�ͷ 
	if(feof(fp) == 0)
	{
		// ��ȡ�ļ�����
		fread(&(bfh.bfType),1,sizeof(bfh.bfType),fp);
		
		// ��ȡ�ļ���С
		fread(&(bfh.bfSize),sizeof(bfh.bfSize),1,fp);
		
		// ��ȡ�ļ�������
		fread(&(bfh.bfReserves),sizeof(bfh.bfReserves),1,fp);
		
		// ��ȡ����ƫ����
		fread(&(bfh.bfOffBits),sizeof(bfh.bfOffBits),1,fp);
	}
	else
	{
		printf("�ļ�������");
		exit(-1);		
	}
	
	// ��֤�ļ�ͷ 
	if(bfh.bfType != 0x4d42 || bfh.bfReserves != 0)
	{
		printf("�ļ��𻵣����ļ�����BMP��ʽ��\n");
		exit(-1);
	}
	
	// ��ȡ��Ϣͷ
	if(feof(fp) == 0)
	{
		// ��ȡ��Ϣͷ����
		fread(&(bi.biSize),1,sizeof(bi.biSize),fp);

		// ��ȡͼƬ���
		fread(&(bi.biWidth),1,sizeof(bi.biWidth),fp);

		// ��ȡͼƬ�߶�
		fread(&(bi.biHeight),1,sizeof(bi.biHeight),fp); 

		// ��ȡͼƬƽ����
		fread(&(bi.biPlanes),1,sizeof(bi.biPlanes),fp);

		// ��ȡͼƬ��ɫλ������ȣ�
		fread(&(bi.biBitCount),1,sizeof(bi.biBitCount),fp);

		// ��ȡͼƬѹ����ʽ
		fread(&(bi.biCompression),1,sizeof(bi.biCompression),fp);

		// ��ȡλͼ���ݳ���
		fread(&(bi.biSizeImages),1,sizeof(bi.biSizeImages),fp);

		// ��ȡˮƽ����ֱ���
		fread(&(bi.biXPelsPerMeter),1,sizeof(bi.biXPelsPerMeter),fp);

		// ��ȡ��ֱ����ֱ���
		fread(&(bi.biYPelsPerMeter),1,sizeof(bi.biYPelsPerMeter),fp);

		// ��ȡʹ����ɫ��
		fread(&(bi.biClrUsed),1,sizeof(bi.biClrUsed),fp);

		// ��ȡ��Ҫ��ɫ��
		fread(&(bi.biClrImportant),1,sizeof(bi.biClrImportant),fp);

	}
	else
	{
		printf("�ļ�������");
		exit(-1);		
	}
	
	// �ж�λͼ�߶����ݵ�����
	int heightFlag = (bi.biHeight<0);
	if(heightFlag == 1)
		bi.biHeight *= -1;
	 
	
	// ������ɫ����������� 
	int colorDictNum = (bfh.bfOffBits - (bfh.bfhSize + bi.biSize))/4;
	
	// ����ļ�����ɫ�������򴴽���ɫ������ 
	if(colorDictNum != 0)
	{
		// ������ɫ����������
		colorDict = (BMP_COLORS*)malloc(colorDictNum * sizeof(BMP_COLORS));
		if(colorDict == NULL)
		{
			printf("��ɫ���������飺�ڴ�ռ俪��ʧ��");
			exit(-1);
		}
		
		// ��ȡ������
		for(i = 0;i<colorDictNum;i++)
		{
			fread(&colorDict[i].Blue,1,1,fp);
			fread(&colorDict[i].Green,1,1,fp);
			fread(&colorDict[i].Red,1,1,fp);
			fread(&colorDict[i].Alpha,1,1,fp);
		}
	}
	
	// ����λͼ���ݾ���
	BMP_COLORS** matrix = (BMP_COLORS**)malloc(bi.biHeight * sizeof(BMP_COLORS*));
	if(matrix == NULL)
	{
		printf("λͼ�������飺�ڴ�ռ俪��ʧ��");
		exit(-1);
	}
	for(i = 0;i < bi.biHeight;i++)
	{
		matrix[i] = (BMP_COLORS*)malloc(bi.biWidth * sizeof(BMP_COLORS));
		if(matrix[i] == NULL)
		{
			printf("λͼ�������飺�ڴ�ռ俪��ʧ��");
			exit(-1);
		}
	}
	
	// ����height������ѡ����ʵ�λͼ�洢��ʽ 
	if(heightFlag == 0)
	{
		// ��ȡλͼ���ݲ���ѹ�洢 
		for(i = bi.biHeight - 1;i >= 0;i--)
		{
			for(j = 0;j < bi.biWidth;j++)
			{
				// ����λ����ֽڵض�ȡ�����ص���ɫ��Ϣ 
				// 8λ����������� 
				if(bi.biBitCount == 8)
				{
					fread(&temp,1,1,fp);
					matrix[i][j] = colorDict[temp];
				}
				// 24λ�����������
				else if(bi.biBitCount ==24)
				{
					fread(&temp,1,1,fp);
					matrix[i][j].Blue = temp;
					fread(&temp,1,1,fp);
					matrix[i][j].Green = temp;
					fread(&temp,1,1,fp);
					matrix[i][j].Red = temp;
				}
				// 32λ�����������
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
			
			// ���ʵ��λͼ���ݴ�С��������˻�����ÿ�����������ֽڡ�
			// ������Ϊ��ͼ���������BMP�ļ���ʱ����ÿ��ĩβ������һ���ֽڵ�0ֵ
			// Ϊ������������ͼ���ȡ�ĸ��ţ�����������ĩβ�ֽ� 
			if(bi.biSizeImages > bi.biHeight * bi.biWidth)
			{
				for(k = 0;k < (bi.biSizeImages - bi.biHeight * bi.biWidth * (bi.biBitCount/8))/bi.biHeight;k++)
					fread(&empty,1,1,fp);
			} 
		}
	}
	else
	{
		// ��ȡλͼ���ݲ���ѹ�洢 
		for(i = 0;i < bi.biHeight ;i++)
		{
			for(j = 0;j < bi.biWidth;j++)
			{
				// ����λ����ֽڵض�ȡ�����ص���ɫ��Ϣ 
				// 8λ�����������
				if(bi.biBitCount == 8)
				{
					fread(&temp,1,1,fp);
					matrix[i][j] = colorDict[temp];
				}
				// 24λ�����������
				else if(bi.biBitCount ==24)
				{
					fread(&temp,1,1,fp);
					matrix[i][j].Blue = temp;
					fread(&temp,1,1,fp);
					matrix[i][j].Green = temp;
					fread(&temp,1,1,fp);
					matrix[i][j].Red = temp;
				}
				// 32λ�����������
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
			
			// ���ʵ��λͼ���ݴ�С��������˻�����ÿ�����������ֽڡ�
			// ������Ϊ��ͼ���������BMP�ļ���ʱ����ÿ��ĩβ������һ���ֽڵ�0ֵ
			// Ϊ������������ͼ���ȡ�ĸ��ţ�����������ĩβ�ֽ� 
			if(bi.biSizeImages > bi.biHeight * bi.biWidth)
			{
				for(k = 0;k < (bi.biSizeImages - bi.biHeight * bi.biWidth * (bi.biBitCount/8))/bi.biHeight;k++)
					fread(&empty,1,1,fp);
			}
		}	
	}
	
	// ��ȡ�Ҷ����ݣ�BGRƽ����
	for(i = 0;i < bi.biHeight;i++)
	{
		for(j = 0;j < bi.biWidth;j++)
			matrix[i][j].Average = int((matrix[i][j].Blue + matrix[i][j].Green + matrix[i][j].Red)/3);
	}
	
	// �ͷ���ɫ�������ڴ� 
	if (colorDictNum > 0)
		free(colorDict);
	
	// �ر��ļ�
	if(fclose(fp) == EOF)
	{
		printf("�ļ��ر�ʧ��");
		exit(-1);
	}
	fp = NULL;
	
	// �����ͼ����� 
	bImage.fileAddress = Address;
	bImage.bfh = bfh;
	bImage.bi = bi;
	bImage.mat = matrix;
	
	return bImage;
}

/* ���BMP�Ҷ�ͼ���ļ���Ϣ */
void printBMPGray(BMP_IMAGE image)
{
	// ����ѭ�����Ʊ��� 
	int i = 0,j = 0; 
	
	// ����ļ���Ϣ
	printf("---------------------------------------------------------------------------------------------------------------------\n");
	printf("%s\n\n",image.fileAddress);
	printf("�ļ���С��%d\n",image.bfh.bfSize);
	printf("�ļ������֣�%d\n",image.bfh.bfReserves);
	printf("����ƫ������%d\n",image.bfh.bfOffBits);
	printf("��Ϣͷ��С��%d\n",image.bi.biSize);
	printf("ͼƬ��ȣ�%d\n",image.bi.biWidth);
	printf("ͼƬ�߶ȣ�%d\n",image.bi.biHeight);
	printf("ͼƬƽ������%d\n",image.bi.biPlanes);
	printf("ͼƬλ�%d\n",image.bi.biBitCount);
	printf("ͼƬѹ����ʽ��%d\n",image.bi.biCompression);
	printf("λͼ���ݴ�С��%d\n",image.bi.biSizeImages);
	printf("ˮƽ����ֱ��ʣ�%d\n",image.bi.biXPelsPerMeter);
	printf("��ֱ����ֱ��ʣ�%d\n",image.bi.biYPelsPerMeter);
	printf("ʹ����ɫ����%d\n",image.bi.biClrUsed);
	printf("��Ҫ��ɫ����%d\n",image.bi.biClrImportant);
	
	// ���ƽ���Ҷ�ͼ
	printf("\nƽ���Ҷ�ͼ\n\n");
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
	
	// �����ɫ�Ҷ�ͼ
	printf("\n��ɫ�Ҷ�ͼ\n\n");
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
	
	// �����ɫ�Ҷ�ͼ
	printf("\n��ɫ�Ҷ�ͼ\n\n");
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
	
	// �����ɫ�Ҷ�ͼ
	printf("\n��ɫ�Ҷ�ͼ\n\n");
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

/* �ڴ��ͷź��� */
void freeBMPMemory(BMP_IMAGE image)
{
	// ����ѭ�����Ʊ���
	int i = 0;
	
	// �ͷ��ڴ� 
	for(i = 0;i < image.bi.biHeight;i++)
		free(image.mat[i]);
	free(image.mat);
	image.mat = NULL;
}

