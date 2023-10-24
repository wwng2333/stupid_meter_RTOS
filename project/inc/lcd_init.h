#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include "at32f421_wk_config.h"

extern __IO uint8_t USE_HORIZONTAL;  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏

//#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
//#define LCD_W 80
//#define LCD_H 160

//#else
#define LCD_W 160
#define LCD_H 80
//#endif



//-----------------LCD端口定义---------------- 

#define LCD_SCLK_Clr() gpio_bits_reset(GPIOA, GPIO_PINS_5) //PA5
#define LCD_SCLK_Set() gpio_bits_set(GPIOA, GPIO_PINS_5)

#define LCD_MOSI_Clr() gpio_bits_reset(GPIOA, GPIO_PINS_7) //PA7
#define LCD_MOSI_Set() gpio_bits_set(GPIOA, GPIO_PINS_7)

#define LCD_RES_Clr()  gpio_bits_reset(GPIOA, GPIO_PINS_3) //PA3
#define LCD_RES_Set()  gpio_bits_set(GPIOA, GPIO_PINS_3)

#define LCD_DC_Clr()   gpio_bits_reset(GPIOA, GPIO_PINS_2) //PA2
#define LCD_DC_Set()   gpio_bits_set(GPIOA, GPIO_PINS_2)
 		     
#define LCD_CS_Clr()   gpio_bits_reset(GPIOA, GPIO_PINS_4) //PA4
#define LCD_CS_Set()   gpio_bits_set(GPIOA, GPIO_PINS_4)

#define LCD_BLK_Clr()  gpio_bits_reset(GPIOA, GPIO_PINS_1) //PA1
#define LCD_BLK_Set()  gpio_bits_set(GPIOA, GPIO_PINS_1)

void LCD_SPI1_init(void);
void LCD_dma1_channel3_init(void);
void wk_dma_channel_config(dma_channel_type* dmax_channely, uint32_t peripheral_base_addr, uint32_t memory_base_addr, uint16_t buffer_size);

void LCD_Writ_Bus(u8 dat);//模拟SPI时序
void LCD_WR_DATA8(u8 dat);//写入一个字节
void LCD_WR_DATA(u16 dat);//写入两个字节
void LCD_WR_REG(u8 dat);//写入一个指令
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2);//设置坐标函数
void LCD_Init(void);//LCD初始化
void LCD_Init_Swap(void);
#endif




