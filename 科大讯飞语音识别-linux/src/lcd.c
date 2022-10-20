#include <stdio.h>   	//printf scanf
#include <fcntl.h>		//open write read lseek close  	 
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include "../include/common.h"
#include "../include/lcd.h"




static char g_color_buf[FB_SIZE]={0};
static int  g_fb_fd;
static int *g_pfb_memory;


int lcd_open(void)
{
	g_fb_fd = open("/dev/ubuntu_lcd", O_RDWR);
	
	if(g_fb_fd<0)
	{
			printf("open lcd error\n");
			return -1;
	}

	g_pfb_memory  = (int *)mmap(	NULL, 					//ӳ�����Ŀ�ʼ��ַ������ΪNULLʱ��ʾ��ϵͳ����ӳ��������ʼ��ַ
									FB_SIZE, 				//ӳ�����ĳ���
									PROT_READ|PROT_WRITE, 	//���ݿ��Ա���ȡ��д��
									MAP_SHARED,				//�����ڴ�
									g_fb_fd, 				//��Ч���ļ�������
									0						//��ӳ��������ݵ����
								);

	return g_fb_fd;

}

void lcd_draw_point(unsigned int x,unsigned int y, unsigned int color)
{
	*(g_pfb_memory+y*800+x)=color;
}

int lcd_draw_bmp(int x, int y,const char *pbmp_path)   
{
			 int bmp_fd;
	unsigned int blue, green, red;
	unsigned int color;
	unsigned int bmp_width;
	unsigned int bmp_height;
	unsigned int bmp_type;
	unsigned int bmp_size;
	int x_s = x;
	int y_s = y;
	int x_e ;	
	int y_e ;
	unsigned int i=0;
	unsigned char buf[54]={0};
	unsigned char *pbmp_buf=NULL;
	unsigned char *ptmp_buf=NULL;
	
	/* �����ڴ�ռ䱣��bmpͼƬ�Ĵ�С */	
	pbmp_buf = (unsigned char *)calloc(1,FB_SIZE);
	
	if(pbmp_buf == NULL)
	{
		printf("calloc bmp buffer error\r\n");
		return -1;
	}
		
	/* ����λͼ��Դ��Ȩ�޿ɶ���д */	
	bmp_fd=open(pbmp_path,O_RDWR);
	
	if(bmp_fd == -1)
	{
	   printf("open bmp error\r\n");
	   
	   return -1;	
	}
	
	
	/* ��ȡλͼͷ����Ϣ */
	read(bmp_fd,buf,54);
	
	/* ����  */
	bmp_width =buf[18];
	bmp_width|=buf[19]<<8;
	printf("bmp_width=%d\r\n",bmp_width);
	
	/* �߶�  */
	bmp_height =buf[22];
	bmp_height|=buf[23]<<8;
	printf("bmp_height=%d\r\n",bmp_height);	
	
	/* �ļ����� */
	bmp_type =buf[28];
	bmp_type|=buf[29]<<8;
	printf("bmp_type=%d\r\n",bmp_type);	

	/* ������ʾx��y�������λ�� */
	x_e = x + bmp_width;
	y_e = y + bmp_height;
	
	/* ��ȡλͼ�ļ��Ĵ�С */
	bmp_size=file_size_get(pbmp_path);
	
	/* ��ȡ����RGB���� */
	read(bmp_fd,pbmp_buf,bmp_size-54);

	/* �����ڴ�ռ�����bmpͼƬ��ˮƽ����ת */	
	ptmp_buf = (unsigned char *)calloc(1,bmp_size);
	
	if(ptmp_buf == NULL)
	{
		printf("calloc bmp buffer error\r\n");
		close(bmp_fd);
		return -1;
	}

	i=0;
	for(y=y_e; y >= y_s; y--)
	{
		for (x=x_s; x < x_e; x++)
		{
			/* ��ȡ��������ɫ���� */
			blue  = *(pbmp_buf+i);
			green = *(pbmp_buf+i+1);
			red   = *(pbmp_buf+i+2);
			
			i+=3;
			
			/* �жϵ�ǰ��λͼ�Ƿ�32λ��ɫ */
			if(bmp_type == 32)
			{
				i++;
			}
			
			/* ���24bit��ɫ */
			color = red << 16 | green << 8 | blue << 0;
			lcd_draw_point(x, y, color);				
		}	
	}
	
	/* �ͷ�������ڴ���Դ */
	free(pbmp_buf);
	free(ptmp_buf);
		
	/* ����ʹ��BMP�����ͷ�bmp��Դ */
	close(bmp_fd);	
	
	return 0;
}

void lcd_fill(unsigned int x,unsigned int y, unsigned int x_len,unsigned int y_len,unsigned int color)
{
	unsigned int x_s=x;
	unsigned int x_e=x+x_len;	
	
	unsigned int y_s=y;
	unsigned int y_e=y+y_len;	
	
	
	
	for(y_s=y; y_s<y_e; y_s++)
		for(x_s=x; x_s<x_e; x_s++)
		{
			lcd_draw_point(x_s,y_s,color);
			
		}
}

void lcd_close(void)
{
	/* ȡ���ڴ�ӳ�� */
	munmap(g_pfb_memory, FB_SIZE);
	
	/* �ر�LCD�豸 */
	close(g_fb_fd);
}