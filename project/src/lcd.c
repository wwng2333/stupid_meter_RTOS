#include "at32f421_wk_config.h"
#include "lcd.h"
#include "Queue.h"
#include "lcd_init.h"
#include "lcdfont.h"
#include <stdbool.h>
#include "w25qxx.h"
#include <stdio.h>

extern bool __SPI_8bit_mode;
extern char sprintf_buf[32];
extern ina226_info_struct ina226_info;
extern ADC_result_struct ADC_result;

void W25Q_WriteFont(void)
{
	uint32_t start_addr = 0x100000;
	uint8_t buf[12] = {0};
	uint8_t size = sizeof(ascii_1206) / sizeof(ascii_1206[0]);
	for (uint8_t i = 0; i < size; i++)
	{
		printf("[lcdfont]start write %x...\r\n", start_addr);
		for (uint8_t j = 0; j < 12; j++)
		{
			buf[j] = ascii_1206[i][j];
		} // read buf from rom
		W25Q_Write(buf, start_addr, 12);
		printf("[lcdfont]write %x done\r\n", start_addr);
		start_addr += 12;
	}
}

void W25Q_CheckFont(void)
{
	uint32_t start_addr = 0x100000;
	uint8_t buf[12] = {0};
	uint8_t size = sizeof(ascii_1206) / sizeof(ascii_1206[0]);
	for (uint8_t i = 0; i < size; i++)
	{
		// printf("[lcdfont]start read %x...\r\n", start_addr);
		W25Q_Read(buf, start_addr, 12);
		for (uint8_t j = 0; j < 12; j++)
		{
			if (buf[j] == ascii_1206[i][j])
			{
				continue;
			}
			else
			{
				printf("[lcdfont]%x err!\r\n", start_addr + j);
				break;
			}
		} // read buf from rom
		printf("[lcdfont]check %x ok\r\n", start_addr);
		start_addr += 12;
	}
	printf("[lcdfont]full check ok\r\n");
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
	  ����˵������ָ�����������ɫ
	  ������ݣ�xsta,ysta   ��ʼ����
				xend,yend   ��ֹ����
								color       Ҫ������ɫ
	  ����ֵ��  ��
******************************************************************************/
void LCD_Fill(u16 xsta, u16 ysta, u16 xend, u16 yend, u16 color)
{
	uint16_t num, _color[1];
	_color[0] = color;
	num = (xend - xsta) * (yend - ysta);
	LCD_Address_Set(xsta, ysta, xend - 1, yend - 1); // ������ʾ��Χ
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
	  ����˵������ָ��λ�û���
	  ������ݣ�x,y ��������
				color �����ɫ
	  ����ֵ��  ��
******************************************************************************/
void LCD_DrawPoint(u16 x, u16 y, u16 color)
{
	LCD_Address_Set(x, y, x, y); // ���ù��λ��
	LCD_WR_DATA(color);
}

/******************************************************************************
	  ����˵��������
	  ������ݣ�x1,y1   ��ʼ����
				x2,y2   ��ֹ����
				color   �ߵ���ɫ
	  ����ֵ��  ��
******************************************************************************/
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2, u16 color)
{
	u16 t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, uRow, uCol;
	delta_x = x2 - x1; // ������������
	delta_y = y2 - y1;
	uRow = x1; // �����������
	uCol = y1;
	if (delta_x > 0)
		incx = 1; // ���õ�������
	else if (delta_x == 0)
		incx = 0; // ��ֱ��
	else
	{
		incx = -1;
		delta_x = -delta_x;
	}
	if (delta_y > 0)
		incy = 1;
	else if (delta_y == 0)
		incy = 0; // ˮƽ��
	else
	{
		incy = -1;
		delta_y = -delta_y;
	}
	if (delta_x > delta_y)
		distance = delta_x; // ѡȡ��������������
	else
		distance = delta_y;
	for (t = 0; t < distance + 1; t++)
	{
		LCD_DrawPoint(uRow, uCol, color); // ����
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
	  ����˵����������
	  ������ݣ�x1,y1   ��ʼ����
				x2,y2   ��ֹ����
				color   ���ε���ɫ
	  ����ֵ��  ��
******************************************************************************/
// void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
//{
//	LCD_DrawLine(x1,y1,x2,y1,color);
//	LCD_DrawLine(x1,y1,x1,y2,color);
//	LCD_DrawLine(x1,y2,x2,y2,color);
//	LCD_DrawLine(x2,y1,x2,y2,color);
// }

/******************************************************************************
	  ����˵������Բ
	  ������ݣ�x0,y0   Բ������
				r       �뾶
				color   Բ����ɫ
	  ����ֵ��  ��
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
//		if((a*a+b*b)>(r*r))//�ж�Ҫ���ĵ��Ƿ��Զ
//		{
//			b--;
//		}
//	}
// }

/******************************************************************************
	  ����˵������ʾ���ִ�
	  ������ݣ�x,y��ʾ����
				*s Ҫ��ʾ�ĺ��ִ�
				fc �ֵ���ɫ
				bc �ֵı���ɫ
				sizey �ֺ� ��ѡ 16 24 32
				mode:  0�ǵ���ģʽ  1����ģʽ
	  ����ֵ��  ��
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
	  ����˵������ʾ����12x12����
	  ������ݣ�x,y��ʾ����
				*s Ҫ��ʾ�ĺ���
				fc �ֵ���ɫ
				bc �ֵı���ɫ
				sizey �ֺ�
				mode:  0�ǵ���ģʽ  1����ģʽ
	  ����ֵ��  ��
******************************************************************************/
// void LCD_ShowChinese12x12(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
//{
//	u8 i,j,m=0;
//	u16 k;
//	u16 HZnum;//������Ŀ
//	u16 TypefaceNum;//һ���ַ���ռ�ֽڴ�С
//	u16 x0=x;
//	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
//
//	HZnum=sizeof(tfont12)/sizeof(typFNT_GB12);	//ͳ�ƺ�����Ŀ
//	for(k=0;k<HZnum;k++)
//	{
//		if((tfont12[k].Index[0]==*(s))&&(tfont12[k].Index[1]==*(s+1)))
//		{
//			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
//			for(i=0;i<TypefaceNum;i++)
//			{
//				for(j=0;j<8;j++)
//				{
//					if(!mode)//�ǵ��ӷ�ʽ
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
//					else//���ӷ�ʽ
//					{
//						if(tfont12[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//��һ����
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
//		continue;  //���ҵ���Ӧ�����ֿ������˳�����ֹ��������ظ�ȡģ����Ӱ��
//	}
// }

/******************************************************************************
	  ����˵������ʾ����16x16����
	  ������ݣ�x,y��ʾ����
				*s Ҫ��ʾ�ĺ���
				fc �ֵ���ɫ
				bc �ֵı���ɫ
				sizey �ֺ�
				mode:  0�ǵ���ģʽ  1����ģʽ
	  ����ֵ��  ��
******************************************************************************/
// void LCD_ShowChinese16x16(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
//{
//	u8 i,j,m=0;
//	u16 k;
//	u16 HZnum;//������Ŀ
//	u16 TypefaceNum;//һ���ַ���ռ�ֽڴ�С
//	u16 x0=x;
//   TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
//	HZnum=sizeof(tfont16)/sizeof(typFNT_GB16);	//ͳ�ƺ�����Ŀ
//	for(k=0;k<HZnum;k++)
//	{
//		if ((tfont16[k].Index[0]==*(s))&&(tfont16[k].Index[1]==*(s+1)))
//		{
//			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
//			for(i=0;i<TypefaceNum;i++)
//			{
//				for(j=0;j<8;j++)
//				{
//					if(!mode)//�ǵ��ӷ�ʽ
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
//					else//���ӷ�ʽ
//					{
//						if(tfont16[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//��һ����
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
//		continue;  //���ҵ���Ӧ�����ֿ������˳�����ֹ��������ظ�ȡģ����Ӱ��
//	}
// }

/******************************************************************************
	  ����˵������ʾ����24x24����
	  ������ݣ�x,y��ʾ����
				*s Ҫ��ʾ�ĺ���
				fc �ֵ���ɫ
				bc �ֵı���ɫ
				sizey �ֺ�
				mode:  0�ǵ���ģʽ  1����ģʽ
	  ����ֵ��  ��
******************************************************************************/
// void LCD_ShowChinese24x24(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
//{
//	u8 i,j,m=0;
//	u16 k;
//	u16 HZnum;//������Ŀ
//	u16 TypefaceNum;//һ���ַ���ռ�ֽڴ�С
//	u16 x0=x;
//	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
//	HZnum=sizeof(tfont24)/sizeof(typFNT_GB24);	//ͳ�ƺ�����Ŀ
//	for(k=0;k<HZnum;k++)
//	{
//		if ((tfont24[k].Index[0]==*(s))&&(tfont24[k].Index[1]==*(s+1)))
//		{
//			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
//			for(i=0;i<TypefaceNum;i++)
//			{
//				for(j=0;j<8;j++)
//				{
//					if(!mode)//�ǵ��ӷ�ʽ
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
//					else//���ӷ�ʽ
//					{
//						if(tfont24[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//��һ����
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
//		continue;  //���ҵ���Ӧ�����ֿ������˳�����ֹ��������ظ�ȡģ����Ӱ��
//	}
// }

/******************************************************************************
	  ����˵������ʾ����32x32����
	  ������ݣ�x,y��ʾ����
				*s Ҫ��ʾ�ĺ���
				fc �ֵ���ɫ
				bc �ֵı���ɫ
				sizey �ֺ�
				mode:  0�ǵ���ģʽ  1����ģʽ
	  ����ֵ��  ��
******************************************************************************/
// void LCD_ShowChinese32x32(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
//{
//	u8 i,j,m=0;
//	u16 k;
//	u16 HZnum;//������Ŀ
//	u16 TypefaceNum;//һ���ַ���ռ�ֽڴ�С
//	u16 x0=x;
//	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
//	HZnum=sizeof(tfont32)/sizeof(typFNT_GB32);	//ͳ�ƺ�����Ŀ
//	for(k=0;k<HZnum;k++)
//	{
//		if ((tfont32[k].Index[0]==*(s))&&(tfont32[k].Index[1]==*(s+1)))
//		{
//			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
//			for(i=0;i<TypefaceNum;i++)
//			{
//				for(j=0;j<8;j++)
//				{
//					if(!mode)//�ǵ��ӷ�ʽ
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
//					else//���ӷ�ʽ
//					{
//						if(tfont32[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//��һ����
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
//		continue;  //���ҵ���Ӧ�����ֿ������˳�����ֹ��������ظ�ȡģ����Ӱ��
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
	  函数说明：显示单个字符
	  入口数据：x,y显示坐标
				num 要显示的字符
				fc 字的颜色
				bc 字的背景色
				sizey 字号
				mode:  0非叠加模式  1叠加模式
	  返回值：  无
******************************************************************************/
void LCD_ShowChar(u16 x, u16 y, u8 num, u16 fc, u16 bc, u8 sizey, u8 mode)
{
	u8 temp, sizex, t, m = 0;
	u16 i, TypefaceNum;
	u16 x0 = x;
	sizex = sizey / 2;
	TypefaceNum = (sizex / 8 + ((sizex % 8) ? 1 : 0)) * sizey;
	num = num - ' ';
	LCD_Address_Set(x, y, x + sizex - 1, y + sizey - 1);
	for (i = 0; i < TypefaceNum; i++)
	{
		if (sizey == 12)
			temp = ascii_1206[num][i];
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
	  ����˵������ʾ�ַ���
	  ������ݣ�x,y��ʾ����
				*p Ҫ��ʾ���ַ���
				fc �ֵ���ɫ
				bc �ֵı���ɫ
				sizey �ֺ�
				mode:  0�ǵ���ģʽ  1����ģʽ
	  ����ֵ��  ��
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
	  ����˵������ʾ����
	  ������ݣ�m������nָ��
	  ����ֵ��  ��
******************************************************************************/
// u32 mypow(u8 m,u8 n)
//{
//	u32 result=1;
//	while(n--)result*=m;
//	return result;
// }

/******************************************************************************
	  ����˵������ʾ��������
	  ������ݣ�x,y��ʾ����
				num Ҫ��ʾ��������
				len Ҫ��ʾ��λ��
				fc �ֵ���ɫ
				bc �ֵı���ɫ
				sizey �ֺ�
	  ����ֵ��  ��
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
	  ����˵������ʾ��λС������
	  ������ݣ�x,y��ʾ����
				num Ҫ��ʾС������
				len Ҫ��ʾ��λ��
				fc �ֵ���ɫ
				bc �ֵı���ɫ
				sizey �ֺ�
	  ����ֵ��  ��
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
	  ����˵������ʾͼƬ
	  ������ݣ�x,y�������
				length ͼƬ����
				width  ͼƬ���
				pic[]  ͼƬ����
	  ����ֵ��  ��
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
