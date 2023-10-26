#include "at32f421_wk_config.h"
#include "lcd_init.h"
#include "lcd.h"
#include "delay.h"
#include "cmsis_os2.h"
#include <stdbool.h>

extern screen_direction_enum screen_direction;  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏
extern bool __SPI_8bit_mode;

/**
  * @brief  init dma1 channel3 for "spi1_tx"
  * @param  none
  * @retval none
  */
void LCD_dma1_channel3_init_byte(void)
{
	/* enable dma1 periph clock */
  crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);
	
  dma_init_type dma_init_struct;

  dma_reset(DMA1_CHANNEL3);
  dma_default_para_init(&dma_init_struct);
  dma_init_struct.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
  dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
  dma_init_struct.memory_inc_enable = TRUE;
  dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
  dma_init_struct.peripheral_inc_enable = FALSE;
  dma_init_struct.priority = DMA_PRIORITY_HIGH;
  dma_init_struct.loop_mode_enable = FALSE;
  dma_init(DMA1_CHANNEL3, &dma_init_struct);
	dma_interrupt_enable(DMA1_CHANNEL3, DMA_FDT_INT, TRUE);
}

/**
  * @brief  init dma1 channel3 for "spi1_tx"
  * @param  none
  * @retval none
  */
void LCD_dma1_channel3_init_halfword(void)
{
	/* enable dma1 periph clock */
  crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);
	
  dma_init_type dma_init_struct;

  dma_reset(DMA1_CHANNEL3);
  dma_default_para_init(&dma_init_struct);
  dma_init_struct.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
  dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_HALFWORD;
  dma_init_struct.memory_inc_enable = FALSE;
  dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_HALFWORD;
  dma_init_struct.peripheral_inc_enable = FALSE;
  dma_init_struct.priority = DMA_PRIORITY_HIGH;
  dma_init_struct.loop_mode_enable = FALSE;
  dma_init(DMA1_CHANNEL3, &dma_init_struct);
	dma_interrupt_enable(DMA1_CHANNEL3, DMA_FDT_INT, TRUE);
}

/**
  * @brief  config dma channel transfer parameter
  * @param  none
  * @retval none
  */
void wk_dma_channel_config(dma_channel_type* dmax_channely, uint32_t peripheral_base_addr, uint32_t memory_base_addr, uint16_t buffer_size)
{
  dmax_channely->dtcnt = buffer_size;
  dmax_channely->paddr = peripheral_base_addr;
  dmax_channely->maddr = memory_base_addr;
}

/**
  * @brief  init spi1 function
  * @param  none
  * @retval none
  */
void LCD_SPI1_init(void)
{
	
  /* add user code begin spi1_init 0 */

  /* add user code end spi1_init 0 */
  /* enable gpioa periph clock */
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);

  /* enable spi1 periph clock */
  crm_periph_clock_enable(CRM_SPI1_PERIPH_CLOCK, TRUE);
	
  gpio_init_type gpio_init_struct;
  spi_init_type spi_init_struct;

	gpio_bits_write(GPIOA, GPIO_PINS_2 | GPIO_PINS_3 | GPIO_PINS_4, TRUE); 
	
  gpio_default_para_init(&gpio_init_struct);
  spi_default_para_init(&spi_init_struct);

  /* add user code begin spi1_init 1 */

  /* add user code end spi1_init 1 */

  /* configure the CS pin */
	//gpio_bits_set(GPIOA, GPIO_PINS_4);
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
  gpio_init_struct.gpio_pins = GPIO_PINS_1 | GPIO_PINS_2 | GPIO_PINS_3 | GPIO_PINS_4;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOA, &gpio_init_struct);
	
  /* configure the SCK pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = GPIO_PINS_5;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOA, &gpio_init_struct);

  gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE5, GPIO_MUX_0);

  /* configure the MOSI pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = GPIO_PINS_7;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOA, &gpio_init_struct);

  gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE7, GPIO_MUX_0);

  /* configure param */
  spi_init_struct.transmission_mode = SPI_TRANSMIT_HALF_DUPLEX_TX;
  spi_init_struct.master_slave_mode = SPI_MODE_MASTER;
  spi_init_struct.frame_bit_num = SPI_FRAME_8BIT;
  spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
  spi_init_struct.mclk_freq_division = SPI_MCLK_DIV_4;
  spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_HIGH;
  spi_init_struct.clock_phase = SPI_CLOCK_PHASE_2EDGE;
  spi_init_struct.cs_mode_selection = SPI_CS_SOFTWARE_MODE;
  spi_init(SPI1, &spi_init_struct);

	//spi_i2s_dma_transmitter_enable(SPI1, TRUE);
  /* configure the cs pin output */

  spi_enable(SPI1, TRUE);

  /* add user code begin spi1_init 2 */

  /* add user code end spi1_init 2 */
}

/******************************************************************************
      函数说明：LCD串行数据写入函数
      入口数据：dat  要写入的串行数据
      返回值：  无
******************************************************************************/
void LCD_Writ_Bus(u8 dat) 
{	
	LCD_CS_Clr();
	spi_i2s_data_transmit(SPI1, dat);
	while (spi_i2s_flag_get(SPI1, SPI_I2S_BF_FLAG) == SET) {};
#ifdef __Crazy_DEBUG
	SEGGER_RTT_printf(0, "SPI1 sent 0x%x\r\n", dat);
#endif
  LCD_CS_Set();	
}


/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA8(u8 dat)
{
	LCD_Writ_Bus(dat);
}


/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA(uint16_t dat)
{
	uint8_t buf[2] = {0};
	buf[0]= dat>>8;
	buf[1]= dat;
#ifdef __USE_SPI_DMA
	if(!__SPI_8bit_mode)
	{
		__SPI_8bit_mode = 1;
		spi_frame_bit_num_set(SPI1, SPI_FRAME_8BIT);
		//spi_i2s_dma_transmitter_enable(SPI1, TRUE);
		spi_i2s_dma_transmitter_enable(SPI1, TRUE);
		LCD_dma1_channel3_init_byte();
	}
	wk_dma_channel_config(DMA1_CHANNEL3, (uint32_t)&SPI1->dt, (uint32_t)buf, 2);
	LCD_CS_Clr();
	dma_channel_enable(DMA1_CHANNEL3, TRUE);
	while (spi_i2s_flag_get(SPI1, SPI_I2S_BF_FLAG) == SET) {};
#else
	LCD_Writ_Bus(buf[0]);
	LCD_Writ_Bus(buf[1]);
#endif
}



/******************************************************************************
      函数说明：LCD写入命令
      入口数据：dat 写入的命令
      返回值：  无
******************************************************************************/
void LCD_WR_REG(u8 dat)
{
	LCD_DC_Clr();//写命令
	LCD_Writ_Bus(dat);
	LCD_DC_Set();//写数据
}


/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2)
{
	switch(screen_direction)
	{
		case SCREEN_VERTICAL:
			LCD_WR_REG(0x2a);//列地址设置
			LCD_WR_DATA(x1+26);
			LCD_WR_DATA(x2+26);
			LCD_WR_REG(0x2b);//行地址设置
			LCD_WR_DATA(y1+1);
			LCD_WR_DATA(y2+1);
			LCD_WR_REG(0x2c);//储存器写
			break;
		case SCREEN_VERTICAL_REVERSED:
			LCD_WR_REG(0x2a);//列地址设置
			LCD_WR_DATA(x1+26);
			LCD_WR_DATA(x2+26);
			LCD_WR_REG(0x2b);//行地址设置
			LCD_WR_DATA(y1+1);
			LCD_WR_DATA(y2+1);
			LCD_WR_REG(0x2c);//储存器写
			break;
		case SCREEN_HORIZONTAL:
			LCD_WR_REG(0x2a);//列地址设置
			LCD_WR_DATA(x1+1);
			LCD_WR_DATA(x2+1);
			LCD_WR_REG(0x2b);//行地址设置
			LCD_WR_DATA(y1+26);
			LCD_WR_DATA(y2+26);
			LCD_WR_REG(0x2c);//储存器写
			break;
		case SCREEN_HORIZONTAL_REVERSED:
			LCD_WR_REG(0x2a);//列地址设置
			LCD_WR_DATA(x1+1);
			LCD_WR_DATA(x2+1);
			LCD_WR_REG(0x2b);//行地址设置
			LCD_WR_DATA(y1+26);
			LCD_WR_DATA(y2+26);
			LCD_WR_REG(0x2c);//储存器写
			break;
	}
}

void LCD_Init(void)
{	
	LCD_RES_Clr();//复位
	osDelay(100);
	LCD_RES_Set();
	osDelay(100);
	
	LCD_BLK_Set();//打开背光
  osDelay(100);
	
	LCD_WR_REG(0x11);     //Sleep out
	osDelay(120);                //Delay 120ms
	LCD_WR_REG(0xB1);     //Normal mode
	LCD_WR_DATA8(0x05);   
	LCD_WR_DATA8(0x3C);   
	LCD_WR_DATA8(0x3C);   
	LCD_WR_REG(0xB2);     //Idle mode
	LCD_WR_DATA8(0x05);   
	LCD_WR_DATA8(0x3C);   
	LCD_WR_DATA8(0x3C);   
	LCD_WR_REG(0xB3);     //Partial mode
	LCD_WR_DATA8(0x05);   
	LCD_WR_DATA8(0x3C);   
	LCD_WR_DATA8(0x3C);   
	LCD_WR_DATA8(0x05);   
	LCD_WR_DATA8(0x3C);   
	LCD_WR_DATA8(0x3C);   
	LCD_WR_REG(0xB4);     //Dot inversion
	LCD_WR_DATA8(0x03);   
	LCD_WR_REG(0xC0);     //AVDD GVDD
	LCD_WR_DATA8(0xAB);   
	LCD_WR_DATA8(0x0B);   
	LCD_WR_DATA8(0x04);   
	LCD_WR_REG(0xC1);     //VGH VGL
	LCD_WR_DATA8(0xC5);   //C0
	LCD_WR_REG(0xC2);     //Normal Mode
	LCD_WR_DATA8(0x0D);   
	LCD_WR_DATA8(0x00);   
	LCD_WR_REG(0xC3);     //Idle
	LCD_WR_DATA8(0x8D);   
	LCD_WR_DATA8(0x6A);   
	LCD_WR_REG(0xC4);     //Partial+Full
	LCD_WR_DATA8(0x8D);   
	LCD_WR_DATA8(0xEE);   
	LCD_WR_REG(0xC5);     //VCOM
	LCD_WR_DATA8(0x0F);   
	LCD_WR_REG(0xE0);     //positive gamma
	LCD_WR_DATA8(0x07);   
	LCD_WR_DATA8(0x0E);   
	LCD_WR_DATA8(0x08);   
	LCD_WR_DATA8(0x07);   
	LCD_WR_DATA8(0x10);   
	LCD_WR_DATA8(0x07);   
	LCD_WR_DATA8(0x02);   
	LCD_WR_DATA8(0x07);   
	LCD_WR_DATA8(0x09);   
	LCD_WR_DATA8(0x0F);   
	LCD_WR_DATA8(0x25);   
	LCD_WR_DATA8(0x36);   
	LCD_WR_DATA8(0x00);   
	LCD_WR_DATA8(0x08);   
	LCD_WR_DATA8(0x04);   
	LCD_WR_DATA8(0x10);   
	LCD_WR_REG(0xE1);     //negative gamma
	LCD_WR_DATA8(0x0A);   
	LCD_WR_DATA8(0x0D);   
	LCD_WR_DATA8(0x08);   
	LCD_WR_DATA8(0x07);   
	LCD_WR_DATA8(0x0F);   
	LCD_WR_DATA8(0x07);   
	LCD_WR_DATA8(0x02);   
	LCD_WR_DATA8(0x07);   
	LCD_WR_DATA8(0x09);   
	LCD_WR_DATA8(0x0F);   
	LCD_WR_DATA8(0x25);   
	LCD_WR_DATA8(0x35);   
	LCD_WR_DATA8(0x00);   
	LCD_WR_DATA8(0x09);   
	LCD_WR_DATA8(0x04);   
	LCD_WR_DATA8(0x10);
		 
	LCD_WR_REG(0xFC);    
	LCD_WR_DATA8(0x80);  
	
	LCD_Init_Swap();
}

void LCD_Init_Swap(void)
{
	LCD_WR_REG(0x3A);     
	LCD_WR_DATA8(0x05);   
	LCD_WR_REG(0x36);
	switch(screen_direction)
	{
		case SCREEN_VERTICAL:
			LCD_WR_DATA8(0x08);
			break;
		case SCREEN_VERTICAL_REVERSED:
			LCD_WR_DATA8(0xC8);
			break;
		case SCREEN_HORIZONTAL:
			LCD_WR_DATA8(0x78);
			break;
		case SCREEN_HORIZONTAL_REVERSED:
			LCD_WR_DATA8(0xA8);
			break;
	}
	LCD_WR_REG(0x21);     //Display inversion
	LCD_WR_REG(0x29);     //Display on
	LCD_WR_REG(0x2A);     //Set Column Address
	LCD_WR_DATA8(0x00);   
	LCD_WR_DATA8(0x1A);  //26  
	LCD_WR_DATA8(0x00);   
	LCD_WR_DATA8(0x69);   //105 
	LCD_WR_REG(0x2B);     //Set Page Address
	LCD_WR_DATA8(0x00);   
	LCD_WR_DATA8(0x01);    //1
	LCD_WR_DATA8(0x00);   
	LCD_WR_DATA8(0xA0);    //160
	LCD_WR_REG(0x2C); 
}






