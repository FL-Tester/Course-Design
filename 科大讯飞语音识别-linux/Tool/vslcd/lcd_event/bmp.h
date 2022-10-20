/*************************************************
 File name : bmp.h
 Create date : 2014-07-02 21:15
 Modified date : 2014-07-02 21:15
 Author : heqingde
 Email : hqd173@163.com
 
 ***********************************************/
#ifndef _BMP_H
#define _BMP_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


#define  FREE(x)       if((x)){free((x));(x)=NULL;}



/**/
#pragma pack(1) //指定为1字节对齐
typedef struct BitMapFileHeader{ //bmp文件信息 14
     unsigned short  bfType;                // the flag of bmp, value is "BM"
     unsigned int    bfSize;                // size BMP file ,unit is unsigned chars
     unsigned int    bfReserved;            // 0
     unsigned int    bfOffBits;             // must be 54
     unsigned int    biSize;            // must be 0x28
     unsigned int    biWidth;           //
     unsigned int    biHeight;          //
     unsigned short     biPlanes;          // must be 1
     unsigned short     biBitCount;            //
     unsigned int    biCompression;         //
     unsigned int    biSizeImage;       //
     unsigned int    biXPelsPerMeter;   //
     unsigned int    biYPelsPerMeter;   //
     unsigned int    biClrUsed;             //
     unsigned int    biClrImportant;        //
}BitMapFileHeader;
#pragma pack(0)

typedef struct BitBmp{
	int bw, bh;
	int pertype ;//一个像素占用的字节数
	unsigned int *startBmp;
}bitBmp;

//根据bmp文件创建图片数据
bitBmp* create_bitBmp_file(const char *filenem);
//创建一个空的bmp图片数据
bitBmp* create_bitBmp_defaut(int w, int h);
//把根据指定的尺寸改变图片大小
bitBmp* zoom_bitBmp(bitBmp* bmp, int wscale, int hscale);
//销毁
bool destroy_bitBmp(bitBmp* bmp);

#endif//_BMP_H
