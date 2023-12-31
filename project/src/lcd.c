#include "at32f421_wk_config.h"
#include "lcd.h"
#include "Queue.h"
#include "lcd_init.h"
#include "lcdfont.h"
//#include <stdbool.h>
//#include "w25qxx.h"
#include "sfud.h"
#include <stdio.h>
#include <string.h>

#define W25_FONT12_START_ADDR (uint32_t)0x3FE000
#define W25_FONT24_START_ADDR (uint32_t)0x3FF000

#pragma clang optimize off

extern uint8_t __SPI_8bit_mode;
extern char sprintf_buf[32];
extern ina226_info_struct ina226_info;
extern ADC_result_struct ADC_result;

void W25Q_WriteFont1206(void)
{
	const sfud_flash *flash = sfud_get_device_table() + SFUD_W25Q32BV_DEVICE_INDEX;
	uint32_t start_addr = W25_FONT12_START_ADDR;
	uint8_t buf_size = 12;
	uint8_t buf[buf_size];
	uint16_t size = sizeof(ascii_1206) / buf_size;
	for (uint8_t i = 0; i < size; i++)
	{
		printf("[font]w,0x%x,%d\r\n", start_addr, buf_size);
		memcpy(buf, ascii_1206[i], buf_size); // read buf from rom
		sfud_write(flash, start_addr, buf_size, buf);
		start_addr += buf_size;
	}
}

void W25Q_WriteFont2412(void)
{
	const sfud_flash *flash = sfud_get_device_table() + SFUD_W25Q32BV_DEVICE_INDEX;
	uint32_t start_addr = W25_FONT24_START_ADDR;
	uint8_t buf_size = 48;
	uint8_t buf[buf_size];
	uint16_t size = sizeof(ascii_2412) / buf_size;
	for (uint8_t i = 0; i < size; i++)
	{
		printf("[font]w,0x%x, %d\r\n", start_addr, buf_size);
		memcpy(buf, ascii_2412[i], buf_size); // read buf from rom
		sfud_write(flash, start_addr, buf_size, buf);
		start_addr += buf_size;
	}
}

void W25Q_CheckFont1206(void)
{
	const sfud_flash *flash = sfud_get_device_table() + SFUD_W25Q32BV_DEVICE_INDEX;
	uint32_t start_addr = W25_FONT12_START_ADDR;
	uint8_t buf_size = 12;
	uint8_t buf[buf_size];
	uint8_t size = sizeof(ascii_1206) / buf_size;
	uint16_t result = 0, err = 0;
	for (uint8_t i = 0; i < size; i++)
	{
		sfud_read(flash, start_addr, buf_size, buf);
		result = strcmp((char *)buf, (char *)ascii_1206[i]);
		if(result) 
		{
			err++;
			printf("[font]check 0x%x failed=%d\r\n", start_addr, result);
		}
		start_addr += buf_size;
	}
	printf("[font]full check done, err=%d\r\n", err);
}

void W25Q_CheckFont2412(void)
{
	const sfud_flash *flash = sfud_get_device_table() + SFUD_W25Q32BV_DEVICE_INDEX;
	uint32_t start_addr = W25_FONT24_START_ADDR;
	uint8_t buf_size = 48;
	uint8_t buf[buf_size];
	uint8_t size = sizeof(ascii_2412) / buf_size;
	uint16_t result = 0, err = 0;
	for (uint8_t i = 0; i < size; i++)
	{
		sfud_read(flash, start_addr, buf_size, buf);
		result = strcmp((char *)buf, (char *)ascii_2412[i]);
		if(result) 
		{
			err++;
			printf("[font]check 0x%x failed=%d\r\n", start_addr, result);
		}
		start_addr += buf_size;
	}
	printf("[font]full check done, err=%d\r\n", err);
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
	  函数说明：在指定区域填充颜色
	  入口数据：xsta,ysta   起始坐标
				xend,yend   终止坐标
								color       要填充的颜色
	  返回值：  无
******************************************************************************/
void LCD_Fill(u16 xsta, u16 ysta, u16 xend, u16 yend, u16 color)
{
	uint16_t num, _color[1];
	_color[0] = color;
	num = (xend - xsta) * (yend - ysta);
	LCD_Address_Set(xsta, ysta, xend - 1, yend - 1); // 设置显示范围
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
	  函数说明：在指定位置画点
	  入口数据：x,y 画点坐标
				color 点的颜色
	  返回值：  无
******************************************************************************/
void LCD_DrawPoint(u16 x, u16 y, u16 color)
{
	LCD_Address_Set(x, y, x, y); // 设置光标位置
	LCD_WR_DATA(color);
}

/******************************************************************************
	  函数说明：画线
	  入口数据：x1,y1   起始坐标
				x2,y2   终止坐标
				color   线的颜色
	  返回值：  无
******************************************************************************/
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2, u16 color)
{
	u16 t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, uRow, uCol;
	delta_x = x2 - x1; // 计算坐标增量
	delta_y = y2 - y1;
	uRow = x1; // 画线起点坐标
	uCol = y1;
	if (delta_x > 0)
		incx = 1; // 设置单步方向
	else if (delta_x == 0)
		incx = 0; // 垂直线
	else
	{
		incx = -1;
		delta_x = -delta_x;
	}
	if (delta_y > 0)
		incy = 1;
	else if (delta_y == 0)
		incy = 0; // 水平线
	else
	{
		incy = -1;
		delta_y = -delta_y;
	}
	if (delta_x > delta_y)
		distance = delta_x; // 选取基本增量坐标轴
	else
		distance = delta_y;
	for (t = 0; t < distance + 1; t++)
	{
		LCD_DrawPoint(uRow, uCol, color); // 画点
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
	  函数说明：画矩形
	  入口数据：x1,y1   起始坐标
				x2,y2   终止坐标
				color   矩形的颜色
	  返回值：  无
******************************************************************************/
// void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
//{
//	LCD_DrawLine(x1,y1,x2,y1,color);
//	LCD_DrawLine(x1,y1,x1,y2,color);
//	LCD_DrawLine(x1,y2,x2,y2,color);
//	LCD_DrawLine(x2,y1,x2,y2,color);
// }

/******************************************************************************
	  函数说明：画圆
	  入口数据：x0,y0   圆心坐标
				r       半径
				color   圆的颜色
	  返回值：  无
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
//		if((a*a+b*b)>(r*r))//判断要画的点是否过远
//		{
//			b--;
//		}
//	}
// }

/******************************************************************************
	  函数说明：显示汉字串
	  入口数据：x,y显示坐标
				*s 要显示的汉字串
				fc 字的颜色
				bc 字的背景色
				sizey 字号 可选 16 24 32
				mode:  0非叠加模式  1叠加模式
	  返回值：  无
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
	  函数说明：显示单个12x12汉字
	  入口数据：x,y显示坐标
				*s 要显示的汉字
				fc 字的颜色
				bc 字的背景色
				sizey 字号
				mode:  0非叠加模式  1叠加模式
	  返回值：  无
******************************************************************************/
// void LCD_ShowChinese12x12(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
//{
//	u8 i,j,m=0;
//	u16 k;
//	u16 HZnum;//汉字数目
//	u16 TypefaceNum;//一个字符所占字节大小
//	u16 x0=x;
//	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
//
//	HZnum=sizeof(tfont12)/sizeof(typFNT_GB12);	//统计汉字数目
//	for(k=0;k<HZnum;k++)
//	{
//		if((tfont12[k].Index[0]==*(s))&&(tfont12[k].Index[1]==*(s+1)))
//		{
//			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
//			for(i=0;i<TypefaceNum;i++)
//			{
//				for(j=0;j<8;j++)
//				{
//					if(!mode)//非叠加方式
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
//					else//叠加方式
//					{
//						if(tfont12[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//画一个点
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
//		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
//	}
// }

/******************************************************************************
	  函数说明：显示单个16x16汉字
	  入口数据：x,y显示坐标
				*s 要显示的汉字
				fc 字的颜色
				bc 字的背景色
				sizey 字号
				mode:  0非叠加模式  1叠加模式
	  返回值：  无
******************************************************************************/
// void LCD_ShowChinese16x16(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
//{
//	u8 i,j,m=0;
//	u16 k;
//	u16 HZnum;//汉字数目
//	u16 TypefaceNum;//一个字符所占字节大小
//	u16 x0=x;
//   TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
//	HZnum=sizeof(tfont16)/sizeof(typFNT_GB16);	//统计汉字数目
//	for(k=0;k<HZnum;k++)
//	{
//		if ((tfont16[k].Index[0]==*(s))&&(tfont16[k].Index[1]==*(s+1)))
//		{
//			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
//			for(i=0;i<TypefaceNum;i++)
//			{
//				for(j=0;j<8;j++)
//				{
//					if(!mode)//非叠加方式
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
//					else//叠加方式
//					{
//						if(tfont16[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//画一个点
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
//		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
//	}
// }

/******************************************************************************
	  函数说明：显示单个24x24汉字
	  入口数据：x,y显示坐标
				*s 要显示的汉字
				fc 字的颜色
				bc 字的背景色
				sizey 字号
				mode:  0非叠加模式  1叠加模式
	  返回值：  无
******************************************************************************/
// void LCD_ShowChinese24x24(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
//{
//	u8 i,j,m=0;
//	u16 k;
//	u16 HZnum;//汉字数目
//	u16 TypefaceNum;//一个字符所占字节大小
//	u16 x0=x;
//	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
//	HZnum=sizeof(tfont24)/sizeof(typFNT_GB24);	//统计汉字数目
//	for(k=0;k<HZnum;k++)
//	{
//		if ((tfont24[k].Index[0]==*(s))&&(tfont24[k].Index[1]==*(s+1)))
//		{
//			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
//			for(i=0;i<TypefaceNum;i++)
//			{
//				for(j=0;j<8;j++)
//				{
//					if(!mode)//非叠加方式
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
//					else//叠加方式
//					{
//						if(tfont24[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//画一个点
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
//		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
//	}
// }

/******************************************************************************
	  函数说明：显示单个32x32汉字
	  入口数据：x,y显示坐标
				*s 要显示的汉字
				fc 字的颜色
				bc 字的背景色
				sizey 字号
				mode:  0非叠加模式  1叠加模式
	  返回值：  无
******************************************************************************/
// void LCD_ShowChinese32x32(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
//{
//	u8 i,j,m=0;
//	u16 k;
//	u16 HZnum;//汉字数目
//	u16 TypefaceNum;//一个字符所占字节大小
//	u16 x0=x;
//	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
//	HZnum=sizeof(tfont32)/sizeof(typFNT_GB32);	//统计汉字数目
//	for(k=0;k<HZnum;k++)
//	{
//		if ((tfont32[k].Index[0]==*(s))&&(tfont32[k].Index[1]==*(s+1)))
//		{
//			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
//			for(i=0;i<TypefaceNum;i++)
//			{
//				for(j=0;j<8;j++)
//				{
//					if(!mode)//非叠加方式
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
//					else//叠加方式
//					{
//						if(tfont32[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//画一个点
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
//		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
//	}
// }

void LCD_ShowChar2416(u16 x, u16 y, u8 num, u16 fc, u16 bc)
{
	const sfud_flash *flash = sfud_get_device_table() + SFUD_W25Q32BV_DEVICE_INDEX;
	u8 sizex, t, m = 0, sizey;
	u16 i; // ??????????
	u16 x0 = x;
	sizex = 16;
	sizey = 24;
	uint8_t buf[48];
	num = num - 32;					   // ???????
	
	if (num)
	{
		//W25Q_Read(buf, W25_FONT24_START_ADDR+(num*48), 48);
		sfud_read(flash, W25_FONT24_START_ADDR+(num*48), 48, buf);
	} 
	else //space
	{
		memset(buf, 0, 48);
	}
	
	LCD_Address_Set(x, y, x + 15, y + 23); // ??????
	for (i = 0; i < 48; i++)
	{
		for (t = 0; t < 8; t++)
		{
			if (buf[i] & (0x01 << t))
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
	  鍑芥暟璇存槑锛氭樉绀哄崟涓瓧绗�
	  鍏ュ彛鏁版嵁锛歺,y鏄剧ず鍧愭爣
				num 瑕佹樉绀虹殑瀛楃
				fc 瀛楃殑棰滆壊
				bc 瀛楃殑鑳屾櫙鑹�
				sizey 瀛楀彿
				mode:  0闈炲彔鍔犳ā寮�  1鍙犲姞妯″紡
	  杩斿洖鍊硷細  鏃�
******************************************************************************/
void LCD_ShowChar(u16 x, u16 y, u8 num, u16 fc, u16 bc, u8 sizey, u8 mode)
{
	const sfud_flash *flash = sfud_get_device_table() + SFUD_W25Q32BV_DEVICE_INDEX;
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
		sfud_read(flash, addr, 12, buf);
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
	  函数说明：显示字符串
	  入口数据：x,y显示坐标
				*p 要显示的字符串
				fc 字的颜色
				bc 字的背景色
				sizey 字号
				mode:  0非叠加模式  1叠加模式
	  返回值：  无
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
	  函数说明：显示数字
	  入口数据：m底数，n指数
	  返回值：  无
******************************************************************************/
// u32 mypow(u8 m,u8 n)
//{
//	u32 result=1;
//	while(n--)result*=m;
//	return result;
// }

/******************************************************************************
	  函数说明：显示整数变量
	  入口数据：x,y显示坐标
				num 要显示整数变量
				len 要显示的位数
				fc 字的颜色
				bc 字的背景色
				sizey 字号
	  返回值：  无
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
	  函数说明：显示两位小数变量
	  入口数据：x,y显示坐标
				num 要显示小数变量
				len 要显示的位数
				fc 字的颜色
				bc 字的背景色
				sizey 字号
	  返回值：  无
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
	  函数说明：显示图片
	  入口数据：x,y起点坐标
				length 图片长度
				width  图片宽度
				pic[]  图片数组
	  返回值：  无
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

#pragma clang optimize on
