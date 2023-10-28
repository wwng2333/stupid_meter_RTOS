#include "at32f421_wk_config.h"
#include "lcd.h"
#include "Queue.h"
#include "lcd_init.h"
#include "lcdfont.h"
#include <stdbool.h>
#include "w25qxx.h"
#include <stdio.h>
#include <string.h>

#define W25_FONT12_START_ADDR 0x110000

extern bool __SPI_8bit_mode;
extern char sprintf_buf[32];
extern ina226_info_struct ina226_info;
extern ADC_result_struct ADC_result;

void W25Q_WriteFont(void)
{
	uint32_t start_addr = W25_FONT12_START_ADDR;
	uint8_t buf_size = sizeof(ascii_1206[0]);
	uint8_t buf[buf_size];
	uint8_t size = sizeof(ascii_1206) / buf_size;
	for (uint8_t i = 0; i < size; i++)
	{
		printf("[lcdfont]start write %x...\r\n", start_addr);
		memcpy(buf, ascii_1206[i], buf_size); // read buf from rom
		W25Q_Write(buf, start_addr, buf_size);
		printf("[lcdfont]write %x done\r\n", start_addr);
		start_addr += buf_size;
	}
}

void W25Q_CheckFont(void)
{
	uint32_t start_addr = W25_FONT12_START_ADDR;
	uint8_t buf_size = 1;
	uint8_t buf[buf_size];
	uint16_t size = sizeof(ascii_1206) / sizeof(ascii_1206[0][0]);
	uint8_t err = 0;
	const unsigned char *p = &ascii_1206[0][0];
	for (uint8_t i = 0; i < size; i++)
	{
		// printf("[lcdfont]start read %x...\r\n", start_addr);
		W25Q_Read(buf, start_addr, 1);
		if(*p == buf[0])
		{
			//printf("[lcdfont]%x ok\r\n", start_addr);
		}
		else
		{
			//printf("[lcdfont]%x err\r\n", start_addr);
			err++;
		}
		p++;
		start_addr++;
	}
	printf("[lcdfont]full check done, err=%d\r\n", err);
}

void LCD_ChartPrint(char flag, char unit, struct Queue *queue)
{
	LCD_DrawLine(0, 14, SIZE, 14, GBLUE);
	LCD_DrawLine(0, 46, SIZE, 46, GBLUE);
	LCD_DrawLine(0, 78, SIZE, 78, GBLUE);
	sprintf(sprintf_buf, "%.1fV %.2fA %.2fW %.1fC   ", ina226_info.Voltage, ina226_info.Current, ina226_info.Power, ADC_result.temp);
	LCD_ShowString(1, 1, sprintf_buf, GBLUE, BLACK, 12, 0);
	LCD_ShowChar(150, 1, flag, GBLUE, BLACK, 12, 0);
	sprintf(sprintf_buf, "0.0%c", unit);
	LCD_ShowString(SIZE + 2, 70, sprintf_buf, GBLUE, BLACK, 12, 0);
	if ((queue->max) / 2 > 10)
	{
		sprintf(sprintf_buf, "%.0f", (queue->max) / 2);
		LCD_ShowString(SIZE + 2, 40, sprintf_buf, GBLUE, BLACK, 12, 0);
	}
	else
	{
		sprintf(sprintf_buf, "%.2f", (queue->max) / 2);
		LCD_ShowString(SIZE + 2, 40, sprintf_buf, GBLUE, BLACK, 12, 0);
	}
	if (queue->max > 10)
	{
		sprintf(sprintf_buf, "%.0f", queue->max);
		LCD_ShowString(SIZE + 2, 13, sprintf_buf, GBLUE, BLACK, 12, 0);
	}
	else
	{
		sprintf(sprintf_buf, "%.2f", queue->max);
		LCD_ShowString(SIZE + 2, 13, sprintf_buf, GBLUE, BLACK, 12, 0);
	}
	ClearPrint();
	printQueue(queue);
}

/******************************************************************************
	  º¯ÊýËµÃ÷£ºÔÚÖ¸¶¨ÇøÓòÌî³äÑÕÉ«
	  Èë¿ÚÊý¾Ý£ºxsta,ysta   ÆðÊ¼×ø±ê
				xend,yend   ÖÕÖ¹×ø±ê
								color       ÒªÌî³äµÄÑÕÉ«
	  ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
void LCD_Fill(u16 xsta, u16 ysta, u16 xend, u16 yend, u16 color)
{
	uint16_t num, _color[1];
	_color[0] = color;
	num = (xend - xsta) * (yend - ysta);
	LCD_Address_Set(xsta, ysta, xend - 1, yend - 1); // ÉèÖÃÏÔÊ¾·¶Î§
	if (__SPI_8bit_mode)
	{
		__SPI_8bit_mode = 0;
		spi_frame_bit_num_set(SPI1, SPI_FRAME_16BIT);
		spi_i2s_dma_transmitter_enable(SPI1, TRUE);
		LCD_dma1_channel3_init_halfword();
	}
	// spi_enable(SPI1, TRUE);
	LCD_CS_Clr();
	wk_dma_channel_config(DMA1_CHANNEL3, (uint32_t)&SPI1->dt, (uint32_t)_color, num);
	dma_channel_enable(DMA1_CHANNEL3, TRUE);
	while (spi_i2s_flag_get(SPI1, SPI_I2S_BF_FLAG))
		;
#ifndef __USE_SPI_DMA
	spi_frame_bit_num_set(SPI1, SPI_FRAME_8BIT);
	spi_i2s_dma_transmitter_enable(SPI1, FALSE);
#endif
	// spi_enable(SPI1, TRUE);
}

/******************************************************************************
	  º¯ÊýËµÃ÷£ºÔÚÖ¸¶¨Î»ÖÃ»­µã
	  Èë¿ÚÊý¾Ý£ºx,y »­µã×ø±ê
				color µãµÄÑÕÉ«
	  ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
void LCD_DrawPoint(u16 x, u16 y, u16 color)
{
	LCD_Address_Set(x, y, x, y); // ÉèÖÃ¹â±êÎ»ÖÃ
	LCD_WR_DATA(color);
}

/******************************************************************************
	  º¯ÊýËµÃ÷£º»­Ïß
	  Èë¿ÚÊý¾Ý£ºx1,y1   ÆðÊ¼×ø±ê
				x2,y2   ÖÕÖ¹×ø±ê
				color   ÏßµÄÑÕÉ«
	  ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2, u16 color)
{
	u16 t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, uRow, uCol;
	delta_x = x2 - x1; // ¼ÆËã×ø±êÔöÁ¿
	delta_y = y2 - y1;
	uRow = x1; // »­ÏßÆðµã×ø±ê
	uCol = y1;
	if (delta_x > 0)
		incx = 1; // ÉèÖÃµ¥²½·½Ïò
	else if (delta_x == 0)
		incx = 0; // ´¹Ö±Ïß
	else
	{
		incx = -1;
		delta_x = -delta_x;
	}
	if (delta_y > 0)
		incy = 1;
	else if (delta_y == 0)
		incy = 0; // Ë®Æ½Ïß
	else
	{
		incy = -1;
		delta_y = -delta_y;
	}
	if (delta_x > delta_y)
		distance = delta_x; // Ñ¡È¡»ù±¾ÔöÁ¿×ø±êÖá
	else
		distance = delta_y;
	for (t = 0; t < distance + 1; t++)
	{
		LCD_DrawPoint(uRow, uCol, color); // »­µã
		xerr += delta_x;
		yerr += delta_y;
		if (xerr > distance)
		{
			xerr -= distance;
			uRow += incx;
		}
		if (yerr > distance)
		{
			yerr -= distance;
			uCol += incy;
		}
	}
}

/******************************************************************************
	  º¯ÊýËµÃ÷£º»­¾ØÐÎ
	  Èë¿ÚÊý¾Ý£ºx1,y1   ÆðÊ¼×ø±ê
				x2,y2   ÖÕÖ¹×ø±ê
				color   ¾ØÐÎµÄÑÕÉ«
	  ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
// void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
//{
//	LCD_DrawLine(x1,y1,x2,y1,color);
//	LCD_DrawLine(x1,y1,x1,y2,color);
//	LCD_DrawLine(x1,y2,x2,y2,color);
//	LCD_DrawLine(x2,y1,x2,y2,color);
// }

/******************************************************************************
	  º¯ÊýËµÃ÷£º»­Ô²
	  Èë¿ÚÊý¾Ý£ºx0,y0   Ô²ÐÄ×ø±ê
				r       °ë¾¶
				color   Ô²µÄÑÕÉ«
	  ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
// void Draw_Circle(u16 x0,u16 y0,u8 r,u16 color)
//{
//	int a,b;
//	a=0;b=r;
//	while(a<=b)
//	{
//		LCD_DrawPoint(x0-b,y0-a,color);             //3
//		LCD_DrawPoint(x0+b,y0-a,color);             //0
//		LCD_DrawPoint(x0-a,y0+b,color);             //1
//		LCD_DrawPoint(x0-a,y0-b,color);             //2
//		LCD_DrawPoint(x0+b,y0+a,color);             //4
//		LCD_DrawPoint(x0+a,y0-b,color);             //5
//		LCD_DrawPoint(x0+a,y0+b,color);             //6
//		LCD_DrawPoint(x0-b,y0+a,color);             //7
//		a++;
//		if((a*a+b*b)>(r*r))//ÅÐ¶ÏÒª»­µÄµãÊÇ·ñ¹ýÔ¶
//		{
//			b--;
//		}
//	}
// }

/******************************************************************************
	  º¯ÊýËµÃ÷£ºÏÔÊ¾ºº×Ö´®
	  Èë¿ÚÊý¾Ý£ºx,yÏÔÊ¾×ø±ê
				*s ÒªÏÔÊ¾µÄºº×Ö´®
				fc ×ÖµÄÑÕÉ«
				bc ×ÖµÄ±³¾°É«
				sizey ×ÖºÅ ¿ÉÑ¡ 16 24 32
				mode:  0·Çµþ¼ÓÄ£Ê½  1µþ¼ÓÄ£Ê½
	  ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
// void LCD_ShowChinese(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
//{
//	while(*s!=0)
//	{
//		if(sizey==12) LCD_ShowChinese12x12(x,y,s,fc,bc,sizey,mode);
//		else if(sizey==16) LCD_ShowChinese16x16(x,y,s,fc,bc,sizey,mode);
//		else if(sizey==24) LCD_ShowChinese24x24(x,y,s,fc,bc,sizey,mode);
//		else if(sizey==32) LCD_ShowChinese32x32(x,y,s,fc,bc,sizey,mode);
//		else return;
//		s+=2;
//		x+=sizey;
//	}
// }

/******************************************************************************
	  º¯ÊýËµÃ÷£ºÏÔÊ¾µ¥¸ö12x12ºº×Ö
	  Èë¿ÚÊý¾Ý£ºx,yÏÔÊ¾×ø±ê
				*s ÒªÏÔÊ¾µÄºº×Ö
				fc ×ÖµÄÑÕÉ«
				bc ×ÖµÄ±³¾°É«
				sizey ×ÖºÅ
				mode:  0·Çµþ¼ÓÄ£Ê½  1µþ¼ÓÄ£Ê½
	  ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
// void LCD_ShowChinese12x12(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
//{
//	u8 i,j,m=0;
//	u16 k;
//	u16 HZnum;//ºº×ÖÊýÄ¿
//	u16 TypefaceNum;//Ò»¸ö×Ö·ûËùÕ¼×Ö½Ú´óÐ¡
//	u16 x0=x;
//	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
//
//	HZnum=sizeof(tfont12)/sizeof(typFNT_GB12);	//Í³¼Æºº×ÖÊýÄ¿
//	for(k=0;k<HZnum;k++)
//	{
//		if((tfont12[k].Index[0]==*(s))&&(tfont12[k].Index[1]==*(s+1)))
//		{
//			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
//			for(i=0;i<TypefaceNum;i++)
//			{
//				for(j=0;j<8;j++)
//				{
//					if(!mode)//·Çµþ¼Ó·½Ê½
//					{
//						if(tfont12[k].Msk[i]&(0x01<<j))LCD_WR_DATA(fc);
//						else LCD_WR_DATA(bc);
//						m++;
//						if(m%sizey==0)
//						{
//							m=0;
//							break;
//						}
//					}
//					else//µþ¼Ó·½Ê½
//					{
//						if(tfont12[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//»­Ò»¸öµã
//						x++;
//						if((x-x0)==sizey)
//						{
//							x=x0;
//							y++;
//							break;
//						}
//					}
//				}
//			}
//		}
//		continue;  //²éÕÒµ½¶ÔÓ¦µãÕó×Ö¿âÁ¢¼´ÍË³ö£¬·ÀÖ¹¶à¸öºº×ÖÖØ¸´È¡Ä£´øÀ´Ó°Ïì
//	}
// }

/******************************************************************************
	  º¯ÊýËµÃ÷£ºÏÔÊ¾µ¥¸ö16x16ºº×Ö
	  Èë¿ÚÊý¾Ý£ºx,yÏÔÊ¾×ø±ê
				*s ÒªÏÔÊ¾µÄºº×Ö
				fc ×ÖµÄÑÕÉ«
				bc ×ÖµÄ±³¾°É«
				sizey ×ÖºÅ
				mode:  0·Çµþ¼ÓÄ£Ê½  1µþ¼ÓÄ£Ê½
	  ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
// void LCD_ShowChinese16x16(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
//{
//	u8 i,j,m=0;
//	u16 k;
//	u16 HZnum;//ºº×ÖÊýÄ¿
//	u16 TypefaceNum;//Ò»¸ö×Ö·ûËùÕ¼×Ö½Ú´óÐ¡
//	u16 x0=x;
//   TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
//	HZnum=sizeof(tfont16)/sizeof(typFNT_GB16);	//Í³¼Æºº×ÖÊýÄ¿
//	for(k=0;k<HZnum;k++)
//	{
//		if ((tfont16[k].Index[0]==*(s))&&(tfont16[k].Index[1]==*(s+1)))
//		{
//			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
//			for(i=0;i<TypefaceNum;i++)
//			{
//				for(j=0;j<8;j++)
//				{
//					if(!mode)//·Çµþ¼Ó·½Ê½
//					{
//						if(tfont16[k].Msk[i]&(0x01<<j))LCD_WR_DATA(fc);
//						else LCD_WR_DATA(bc);
//						m++;
//						if(m%sizey==0)
//						{
//							m=0;
//							break;
//						}
//					}
//					else//µþ¼Ó·½Ê½
//					{
//						if(tfont16[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//»­Ò»¸öµã
//						x++;
//						if((x-x0)==sizey)
//						{
//							x=x0;
//							y++;
//							break;
//						}
//					}
//				}
//			}
//		}
//		continue;  //²éÕÒµ½¶ÔÓ¦µãÕó×Ö¿âÁ¢¼´ÍË³ö£¬·ÀÖ¹¶à¸öºº×ÖÖØ¸´È¡Ä£´øÀ´Ó°Ïì
//	}
// }

/******************************************************************************
	  º¯ÊýËµÃ÷£ºÏÔÊ¾µ¥¸ö24x24ºº×Ö
	  Èë¿ÚÊý¾Ý£ºx,yÏÔÊ¾×ø±ê
				*s ÒªÏÔÊ¾µÄºº×Ö
				fc ×ÖµÄÑÕÉ«
				bc ×ÖµÄ±³¾°É«
				sizey ×ÖºÅ
				mode:  0·Çµþ¼ÓÄ£Ê½  1µþ¼ÓÄ£Ê½
	  ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
// void LCD_ShowChinese24x24(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
//{
//	u8 i,j,m=0;
//	u16 k;
//	u16 HZnum;//ºº×ÖÊýÄ¿
//	u16 TypefaceNum;//Ò»¸ö×Ö·ûËùÕ¼×Ö½Ú´óÐ¡
//	u16 x0=x;
//	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
//	HZnum=sizeof(tfont24)/sizeof(typFNT_GB24);	//Í³¼Æºº×ÖÊýÄ¿
//	for(k=0;k<HZnum;k++)
//	{
//		if ((tfont24[k].Index[0]==*(s))&&(tfont24[k].Index[1]==*(s+1)))
//		{
//			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
//			for(i=0;i<TypefaceNum;i++)
//			{
//				for(j=0;j<8;j++)
//				{
//					if(!mode)//·Çµþ¼Ó·½Ê½
//					{
//						if(tfont24[k].Msk[i]&(0x01<<j))LCD_WR_DATA(fc);
//						else LCD_WR_DATA(bc);
//						m++;
//						if(m%sizey==0)
//						{
//							m=0;
//							break;
//						}
//					}
//					else//µþ¼Ó·½Ê½
//					{
//						if(tfont24[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//»­Ò»¸öµã
//						x++;
//						if((x-x0)==sizey)
//						{
//							x=x0;
//							y++;
//							break;
//						}
//					}
//				}
//			}
//		}
//		continue;  //²éÕÒµ½¶ÔÓ¦µãÕó×Ö¿âÁ¢¼´ÍË³ö£¬·ÀÖ¹¶à¸öºº×ÖÖØ¸´È¡Ä£´øÀ´Ó°Ïì
//	}
// }

/******************************************************************************
	  º¯ÊýËµÃ÷£ºÏÔÊ¾µ¥¸ö32x32ºº×Ö
	  Èë¿ÚÊý¾Ý£ºx,yÏÔÊ¾×ø±ê
				*s ÒªÏÔÊ¾µÄºº×Ö
				fc ×ÖµÄÑÕÉ«
				bc ×ÖµÄ±³¾°É«
				sizey ×ÖºÅ
				mode:  0·Çµþ¼ÓÄ£Ê½  1µþ¼ÓÄ£Ê½
	  ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
// void LCD_ShowChinese32x32(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
//{
//	u8 i,j,m=0;
//	u16 k;
//	u16 HZnum;//ºº×ÖÊýÄ¿
//	u16 TypefaceNum;//Ò»¸ö×Ö·ûËùÕ¼×Ö½Ú´óÐ¡
//	u16 x0=x;
//	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
//	HZnum=sizeof(tfont32)/sizeof(typFNT_GB32);	//Í³¼Æºº×ÖÊýÄ¿
//	for(k=0;k<HZnum;k++)
//	{
//		if ((tfont32[k].Index[0]==*(s))&&(tfont32[k].Index[1]==*(s+1)))
//		{
//			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
//			for(i=0;i<TypefaceNum;i++)
//			{
//				for(j=0;j<8;j++)
//				{
//					if(!mode)//·Çµþ¼Ó·½Ê½
//					{
//						if(tfont32[k].Msk[i]&(0x01<<j))LCD_WR_DATA(fc);
//						else LCD_WR_DATA(bc);
//						m++;
//						if(m%sizey==0)
//						{
//							m=0;
//							break;
//						}
//					}
//					else//µþ¼Ó·½Ê½
//					{
//						if(tfont32[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//»­Ò»¸öµã
//						x++;
//						if((x-x0)==sizey)
//						{
//							x=x0;
//							y++;
//							break;
//						}
//					}
//				}
//			}
//		}
//		continue;  //²éÕÒµ½¶ÔÓ¦µãÕó×Ö¿âÁ¢¼´ÍË³ö£¬·ÀÖ¹¶à¸öºº×ÖÖØ¸´È¡Ä£´øÀ´Ó°Ïì
//	}
// }

void LCD_ShowChar2416(u16 x, u16 y, u8 num, u16 fc, u16 bc)
{
	u8 sizex, t, m = 0, sizey;
	u16 i; // ??????????
	u16 x0 = x;
	sizex = 16;
	sizey = 24;
	// TypefaceNum=48;
	num = num - ' ';					   // ???????
	LCD_Address_Set(x, y, x + 15, y + 23); // ??????
	for (i = 0; i < 48; i++)
	{
		for (t = 0; t < 8; t++)
		{
			if (ascii_2412[num][i] & (0x01 << t))
				LCD_WR_DATA(fc);
			else
				LCD_WR_DATA(bc);
			m++;
			if ((m & (sizex - 1)) == 0)
			{
				m = 0;
				break;
			}
		}
	}
}

void LCD_ShowString2416(u16 x, u16 y, char *p, u16 fc, u16 bc)
{
	while (*p != '\0')
	{
		if (*p == '.')
		{
			LCD_ShowCharDot(x, y, fc, bc);
			x += 5;
		}
		else
		{
			LCD_ShowChar2416(x, y, *p, fc, bc);
			x += 16;
		}
		p++;
	}
}

void LCD_ShowCharDot(u16 x, u16 y, u16 fc, u16 bc)
{
	u8 t, m = 0;
	u16 i;								  // ??????????
	LCD_Address_Set(x, y, x + 7, y + 23); // ??????
	for (i = 0; i < 24; i++)
	{
		for (t = 0; t < 8; t++)
		{
			if (dot[i] & (0x01 << t))
				LCD_WR_DATA(fc);
			else
				LCD_WR_DATA(bc);
		}
	}
}

/******************************************************************************
	  å‡½æ•°è¯´æ˜Žï¼šæ˜¾ç¤ºå•ä¸ªå­—ç¬¦
	  å…¥å£æ•°æ®ï¼šx,yæ˜¾ç¤ºåæ ‡
				num è¦æ˜¾ç¤ºçš„å­—ç¬¦
				fc å­—çš„é¢œè‰²
				bc å­—çš„èƒŒæ™¯è‰²
				sizey å­—å·
				mode:  0éžå åŠ æ¨¡å¼  1å åŠ æ¨¡å¼
	  è¿”å›žå€¼ï¼š  æ— 
******************************************************************************/
void LCD_ShowChar(u16 x, u16 y, u8 num, u16 fc, u16 bc, u8 sizey, u8 mode)
{
	uint8_t buf[sizey];
	u8 temp, sizex, t, m = 0;
	u16 i, TypefaceNum;
	u16 x0 = x;
	sizex = sizey / 2;
	TypefaceNum = (sizex / 8 + ((sizex % 8) ? 1 : 0)) * sizey;
	num = num - 32;
	LCD_Address_Set(x, y, x + sizex - 1, y + sizey - 1);
	
	if (sizey == 12 && num)
	{
		uint32_t addr = W25_FONT12_START_ADDR+num*12;
		W25Q_Read(buf, addr, sizey);
	} 
	else if (num == 0) //space
	{
		memset(buf, 0, sizey);
	}
	
	for (i = 0; i < TypefaceNum; i++)
	{
		if (sizey == 12)
		{
			temp = buf[i];
			//temp = ascii_1206[num][i];
		}
		else if (sizey == 16)
			temp = ascii_1608[num][i];
		else if (sizey == 24)
			temp = ascii_2412[num][i];
		// else if(sizey==32)temp=ascii_3216[num][i];
		else
			return;
		for (t = 0; t < 8; t++)
		{
			if (!mode)
			{
				if (temp & (0x01 << t))
					LCD_WR_DATA(fc);
				else
					LCD_WR_DATA(bc);
				m++;
				if (m % sizex == 0)
				{
					m = 0;
					break;
				}
			}
			else
			{
				if (temp & (0x01 << t))
					LCD_DrawPoint(x, y, fc);
				x++;
				if ((x - x0) == sizex)
				{
					x = x0;
					y++;
					break;
				}
			}
		}
	}
}

/******************************************************************************
	  º¯ÊýËµÃ÷£ºÏÔÊ¾×Ö·û´®
	  Èë¿ÚÊý¾Ý£ºx,yÏÔÊ¾×ø±ê
				*p ÒªÏÔÊ¾µÄ×Ö·û´®
				fc ×ÖµÄÑÕÉ«
				bc ×ÖµÄ±³¾°É«
				sizey ×ÖºÅ
				mode:  0·Çµþ¼ÓÄ£Ê½  1µþ¼ÓÄ£Ê½
	  ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
void LCD_ShowString(u16 x, u16 y, char *p, u16 fc, u16 bc, u8 sizey, u8 mode)
{
	while (*p != '\0')
	{
		LCD_ShowChar(x, y, *p, fc, bc, sizey, mode);
		x += sizey / 2;
		p++;
	}
}

/******************************************************************************
	  º¯ÊýËµÃ÷£ºÏÔÊ¾Êý×Ö
	  Èë¿ÚÊý¾Ý£ºmµ×Êý£¬nÖ¸Êý
	  ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
// u32 mypow(u8 m,u8 n)
//{
//	u32 result=1;
//	while(n--)result*=m;
//	return result;
// }

/******************************************************************************
	  º¯ÊýËµÃ÷£ºÏÔÊ¾ÕûÊý±äÁ¿
	  Èë¿ÚÊý¾Ý£ºx,yÏÔÊ¾×ø±ê
				num ÒªÏÔÊ¾ÕûÊý±äÁ¿
				len ÒªÏÔÊ¾µÄÎ»Êý
				fc ×ÖµÄÑÕÉ«
				bc ×ÖµÄ±³¾°É«
				sizey ×ÖºÅ
	  ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
// void LCD_ShowIntNum(u16 x,u16 y,u16 num,u8 len,u16 fc,u16 bc,u8 sizey)
//{
//	u8 t,temp;
//	u8 enshow=0;
//	u8 sizex=sizey/2;
//	for(t=0;t<len;t++)
//	{
//		temp=(num/mypow(10,len-t-1))%10;
//		if(enshow==0&&t<(len-1))
//		{
//			if(temp==0)
//			{
//				LCD_ShowChar(x+t*sizex,y,' ',fc,bc,sizey,0);
//				continue;
//			}else enshow=1;
//
//		}
//	 	LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
//	}
// }

/******************************************************************************
	  º¯ÊýËµÃ÷£ºÏÔÊ¾Á½Î»Ð¡Êý±äÁ¿
	  Èë¿ÚÊý¾Ý£ºx,yÏÔÊ¾×ø±ê
				num ÒªÏÔÊ¾Ð¡Êý±äÁ¿
				len ÒªÏÔÊ¾µÄÎ»Êý
				fc ×ÖµÄÑÕÉ«
				bc ×ÖµÄ±³¾°É«
				sizey ×ÖºÅ
	  ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
// void LCD_ShowFloatNum1(u16 x,u16 y,float num,u8 len,u16 fc,u16 bc,u8 sizey)
//{
//	u8 t,temp,sizex;
//	u16 num1;
//	sizex=sizey/2;
//	num1=num*100;
//	for(t=0;t<len;t++)
//	{
//		temp=(num1/mypow(10,len-t-1))%10;
//		if(t==(len-2))
//		{
//			LCD_ShowChar(x+(len-2)*sizex,y,'.',fc,bc,sizey,0);
//			t++;
//			len+=1;
//		}
//	 	LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
//	}
// }

/******************************************************************************
	  º¯ÊýËµÃ÷£ºÏÔÊ¾Í¼Æ¬
	  Èë¿ÚÊý¾Ý£ºx,yÆðµã×ø±ê
				length Í¼Æ¬³¤¶È
				width  Í¼Æ¬¿í¶È
				pic[]  Í¼Æ¬Êý×é
	  ·µ»ØÖµ£º  ÎÞ
******************************************************************************/
// void LCD_ShowPicture(u16 x,u16 y,u16 length,u16 width,const u8 pic[])
//{
//	u16 i,j;
//	u32 k=0;
//	LCD_Address_Set(x,y,x+length-1,y+width-1);
//	for(i=0;i<length;i++)
//	{
//		for(j=0;j<width;j++)
//		{
//			LCD_WR_DATA8(pic[k*2]);
//			LCD_WR_DATA8(pic[k*2+1]);
//			k++;
//		}
//	}
// }
