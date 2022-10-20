#pragma once

#define LCD_WIDTH  			800
#define LCD_HEIGHT 			480
#define FB_SIZE				(LCD_WIDTH * LCD_HEIGHT * 4)

extern int 	lcd_open(void);
extern void lcd_draw_point(unsigned int x,unsigned int y, unsigned int color);
extern void lcd_close(void);
extern void lcd_fill(unsigned int x,unsigned int y, unsigned int x_len,unsigned int y_len,unsigned int color);
extern int  lcd_draw_bmp( int x, int y,const char *pbmp_path);  