#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

/* BMP�ļ�ͷ */ 
typedef struct
{
	// �ļ�ͷ��С
	int bfhSize; 
	 
	// �ļ����� 
	short bfType; 
	
	// �ļ���С
	int bfSize;
	
	// ������
	int bfReserves;
	
	// λͼ����ƫ��
	int bfOffBits;
	 
}BMP_FILE_HEADER;


/* BMPλͼ��Ϣͷ */
typedef struct
{
	// ��Ϣͷ��С
	int biSize;
	
	// ͼ����
	int biWidth;
	
	// ͼ��߶�
	int biHeight; 
	
	// ƽ����(��Ϊ1)
	short biPlanes;
	
	// ��ɫλ��
	short biBitCount;
	
	// ѹ������
	int biCompression;
	
	// λͼ���ݴ�С
	int biSizeImages;
	
	// ˮƽ�ֱ���
	int biXPelsPerMeter;
	
	// ��ֱ�ֱ���
	int biYPelsPerMeter;
	
	// ʹ�õ���ɫ��
	int biClrUsed;
	
	// ��Ҫ����ɫ��
	int biClrImportant;
	 
}BITMAP_INFORMATION;


/* �����ص���ɫ */
typedef struct
{
	unsigned char Blue;
	unsigned char Green;
	unsigned char Red;
	unsigned char Alpha;
	unsigned char Average;
	
}BMP_COLORS;

/* ����ͼ��ṹ�� */
typedef struct
{
	// �ļ���ַ
	char* fileAddress;
	 
	// �ļ�ͷ 
	BMP_FILE_HEADER bfh;
	
	// λͼ��Ϣͷ 
	BITMAP_INFORMATION bi;
	
	// λͼ���� 
	BMP_COLORS** mat = NULL;
	
}BMP_IMAGE;


/* BMP�ļ���ȡ���� */
BMP_IMAGE BMP_Read(char* Address); 

/* ���BMP�Ҷ�ͼ���ļ���Ϣ */
void printBMPGray(BMP_IMAGE image);

/* �ͷ�λͼ�����ڴ� */
void freeBMPMemory(BMP_IMAGE image);
