#ifndef __LCD_H
#define __LCD_H		

#include "at32f421_wk_config.h"
#include "Queue.h"

void W25Q_WriteFont1206(void);
void W25Q_WriteFont2412(void);
void W25Q_CheckFont1206(void);
void W25Q_CheckFont2412(void);

void LCD_ChartPrint(char flag, char unit, struct Queue* queue);

void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color);
void LCD_DrawPoint(u16 x,u16 y,u16 color);
void LCD_DrawLine(u16 x1,u16 y1,u16 x2,u16 y2,u16 color);
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color);
void Draw_Circle(u16 x0,u16 y0,u8 r,u16 color);

void LCD_ShowChar2416(u16 x, u16 y, u8 num, u16 fc, u16 bc);
void LCD_ShowString2416(u16 x, u16 y, char *p, u16 fc, u16 bc);
void LCD_ShowCharDot(u16 x, u16 y, u16 fc, u16 bc);

void LCD_ShowChar(u16 x,u16 y,u8 num,u16 fc,u16 bc,u8 sizey,u8 mode);
void LCD_ShowString(u16 x,u16 y,char *p,u16 fc,u16 bc,u8 sizey,u8 mode);

#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE           	 0x001F  
#define BRED             0XF81F
#define GRED 			       0XFFE0
#define GBLUE			       0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			     0XBC40
#define BRRED 			     0XFC07
#define GRAY  			     0X8430
#define DARKBLUE      	 0X01CF
#define LIGHTBLUE      	 0X7D7C
#define GRAYBLUE       	 0X5458
#define LIGHTGREEN     	 0X841F
#define LGRAY 			     0XC618
#define LGRAYBLUE        0XA651
#define LBBLUE           0X2B12

#endif





