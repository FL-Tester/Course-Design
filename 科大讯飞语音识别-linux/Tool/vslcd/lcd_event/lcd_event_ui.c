#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <linux/input.h>
#include "bmp.h"

//设计一个lcd结构体
typedef struct LcdDevice{
	int fd;
	unsigned int *mmp;
	int lw, lh;
	int pertype;
}LcdDevice;


//初始化lcd设备
LcdDevice *create_lcd(const char *devname)
{
	LcdDevice *lcd = malloc(sizeof(LcdDevice));
	//打开设备
	lcd->fd = open(devname, O_RDWR);
	if(lcd->fd  < 0)
	{
		perror("open fail");
		free(lcd);
		return NULL;
	}
	//映射空间
	lcd->mmp = (unsigned int *)mmap(NULL, 800*480*4, PROT_READ|PROT_WRITE,
			MAP_SHARED, lcd->fd, 0);
	if(lcd->mmp ==  (void *)-1)
	{
		perror("mmap fail");
		close(lcd->fd);
		free(lcd);
		return NULL;
	}
	//初始lcd参数：宽， 高， 深度
	lcd->lw = 800;
	lcd->lh = 480;
	lcd->pertype = 4;
	return lcd;
}

void draw_lcd(LcdDevice* lcd, int x, int y, bitBmp *bmp)
{
	unsigned int *p = lcd->mmp+y*lcd->lw+x;
	unsigned int *pic = bmp->startBmp;
	for(int i=0; i<bmp->bh; i++)
	{
		memcpy(p, pic, bmp->bw*bmp->pertype);
		p+=lcd->lw;
		pic+=bmp->bw;	
	}
}

//销毁lcd设备
bool destroy_lcd(LcdDevice *lcd)
{
	if(lcd == NULL) return false;
	//释放映射
	munmap(lcd->mmp, 800*480*4);
	//关闭设备
	close(lcd->fd);
	free(lcd);
	return true;
}

//清屏
void clear_lcd(LcdDevice *lcd, unsigned int color)
{
	unsigned int *p = lcd->mmp;
	for(int i=0; i<480*800; i++)
	{
		p[i] = color;
	}
}

void get_xy(int fd, int *x, int *y)
{
	*x=-1;
	*y=-1;
	//2.读取数据
	struct input_event data;
	while(1)
	{
		int size = read(fd, &data, sizeof(data));
		if(size < 0)
		{
			perror("read fail");
			exit(1);
		}

		//printf("type=%hd code=%hd value=%d\n", data.type, data.code, data.value);
		if(data.type == 3 && data.code ==0)//x坐标
		{
			*x = data.value;
		}else if(data.type == 3 && data.code ==1)//y坐标
		{	
			*y = data.value;
			if(*x >= 0)
			{
				break;
			}
		}
	}
	
	//把1024*600的坐标换算800*480坐标
	//*x=(*x)*800/1024;
	//*y=(*y)*480/600;
}

int main(int argc , char **argv)
{
	LcdDevice* lcd = create_lcd("/dev/ubuntu_lcd");
	//LcdDevice* lcd = create_lcd("/dev/fb0");
	if(lcd == NULL) exit(1);

	
	//触摸屏
	//1.打开设备/dev/input/event0
	//int fd = open("/dev/input/event0", O_RDWR);
	int fd = open("/dev/ubuntu_event", O_RDWR);
	if(fd < 0)
	{
		perror("open fail");
		exit(1);
	}
	clear_lcd(lcd, 0xffffff);//把屏幕设置白色背景
	
	//创建bmp图片数据
	bitBmp *bmp = create_bitBmp_file("./led.bmp");
	draw_lcd(lcd, 100,100, bmp);
	destroy_bitBmp(bmp);
	
	int flag = 0;
	
	int x,y;
	while(1)
	{
		get_xy(fd , &x, &y);
		if(x >100 && x<228 && y>100 && y<228)
		{
			if(flag == 0)
			{
				bitBmp *bmp = create_bitBmp_file("./led-2.bmp");
				draw_lcd(lcd, 100,100, bmp);
				destroy_bitBmp(bmp);
				flag = 1;
			}else
			{
				bitBmp *bmp = create_bitBmp_file("./led.bmp");
				draw_lcd(lcd, 100,100, bmp);
				destroy_bitBmp(bmp);
				flag = 0;
			}
		}
	}
	
	destroy_lcd(lcd);
	close(fd);
	return 0;
}