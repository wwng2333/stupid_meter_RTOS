#include "at32f421_wk_config.h"
#include "w25qxx.h"

uint8_t W25Q_Buffer[4096] = {0};

void W25Q_SPI2_Init(void)
{
	crm_periph_clock_enable(CRM_SPI2_PERIPH_CLOCK, TRUE);
	gpio_init_type gpio_init_struct;
	gpio_default_para_init(&gpio_init_struct);

	/* configure the SCK pin */
	gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
	gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
	gpio_init_struct.gpio_pins = GPIO_PINS_1;
	gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
	gpio_init(GPIOB, &gpio_init_struct);

	gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE1, GPIO_MUX_6);

	/* configure the MISO pin */
	gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
	gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
	gpio_init_struct.gpio_pins = GPIO_PINS_4;
	gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
	gpio_init(GPIOB, &gpio_init_struct);

	gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE4, GPIO_MUX_6);

	/* configure the MOSI pin */
	gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
	gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
	gpio_init_struct.gpio_pins = GPIO_PINS_5;
	gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
	gpio_init(GPIOB, &gpio_init_struct);

	gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE5, GPIO_MUX_6);
	
	/* configure the CS pin */
	gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
	gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
	gpio_init_struct.gpio_pins = GPIO_PINS_15;
	gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
	gpio_init(GPIOA, &gpio_init_struct);
	gpio_bits_write(GPIOA, GPIO_PINS_15, FALSE);
	
  spi_init_type spi_init_struct;
  spi_default_para_init(&spi_init_struct);
	
  /* configure param */
  spi_init_struct.transmission_mode = SPI_TRANSMIT_FULL_DUPLEX;
  spi_init_struct.master_slave_mode = SPI_MODE_MASTER;
  spi_init_struct.frame_bit_num = SPI_FRAME_8BIT;
  spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
  spi_init_struct.mclk_freq_division = SPI_MCLK_DIV_4;
  spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_HIGH;
  spi_init_struct.clock_phase = SPI_CLOCK_PHASE_2EDGE;
  spi_init_struct.cs_mode_selection = SPI_CS_SOFTWARE_MODE;
  spi_init(SPI2, &spi_init_struct);

  //spi_crc_polynomial_set(SPI2, 0x7);
  //spi_crc_enable(SPI2, FALSE);

  spi_enable(SPI2, TRUE);
}	

void W25Q_Write(uint8_t* pBuffer, uint32_t addr, uint16_t size)
{
	uint32_t sec_pos;
	uint16_t sec_offset, sec_remain, i;
	uint8_t *RamBuffer;
	RamBuffer = W25Q_Buffer;
	sec_pos = addr / 4096;
	sec_offset = addr % 4096;
	sec_remain = 4096 - sec_offset;
	SEGGER_RTT_printf(0, "Write 1st addr: 0x%x, size: %d\r\n", addr, size);
	if(size < sec_remain) sec_remain = size;
	while(1)
	{
		W25Q_Read(RamBuffer, sec_pos * 4096, 4096);
		for(i=0; i<sec_remain; i++)
		{
			if(RamBuffer[sec_offset+i] != 0xFF) break; //need erase
		}
		if(i < sec_remain) //need erase
		{
			W25Q_EraseSector(sec_pos);
			for(i=0; i<sec_remain; i++) //copy
			{
				RamBuffer[i] = pBuffer[i];
			}
			W25Q_NoCheckWrite(RamBuffer, sec_pos * 4096, 4096); //full sector write
		}
		else
		{
			W25Q_NoCheckWrite(pBuffer, addr, sec_remain);
		}
		if(size == sec_remain) // write done.
		{
			break;
		}
		else // still need write
		{
			sec_pos++;
			sec_offset = 0;
			pBuffer+= sec_remain;
			addr += sec_remain;
			size -= sec_remain;
			if(size > 4096) 
			{
				sec_remain = 4096; //need more sectors
			}
			else 
			{
				sec_remain = size; //last sector finish
			}
		}
	}
	SEGGER_RTT_printf(0, "W25Q32 write finish!\r\n");
}

void W25Q_NoCheckWrite(uint8_t* pBuffer, uint32_t addr, uint16_t size)
{
	uint16_t page_remain;
	page_remain = 256-addr%256;
	if(size <= page_remain) page_remain = size;
	while(1)
	{
		W25Q_WritePage(pBuffer, addr, size);
		if(page_remain == size) 
		{
			break;
		} 
		else 
		{
			pBuffer += page_remain;
			addr += page_remain;
			size -= page_remain;
			if(size > 256) 
			{
				page_remain = 256;
			}
			else 
			{
				page_remain = size;
			}
		}
	}
}

void W25Q_WritePage(uint8_t* pBuffer, uint32_t addr, uint16_t size)
{
	SEGGER_RTT_printf(0, "WritePage addr: 0x%x\r\n", addr);
	uint16_t i;
	W25Q_EnableWrite();
	W25Q_CS_Clr();
	W25Q_ReadWriteByte(W25X_PageProgram);
	W25Q_ReadWriteByte((uint8_t)(addr >> 16));
	W25Q_ReadWriteByte((uint8_t)(addr >> 8));
	W25Q_ReadWriteByte((uint8_t)addr);
	for(i=0; i<size; i++)
	{
		W25Q_ReadWriteByte(pBuffer[i]);
	}
	W25Q_CS_Set();
	W25Q_Wait();
}

void W25Q_EraseSector(uint32_t addr)
{
	SEGGER_RTT_printf(0, "EraseSector addr: 0x%x\r\n", addr);
	addr *= 4096;
	W25Q_EnableWrite();
	W25Q_Wait();
	W25Q_CS_Clr();
	W25Q_ReadWriteByte(W25X_SectorErase);
	W25Q_ReadWriteByte((uint8_t)(addr >> 16));
	W25Q_ReadWriteByte((uint8_t)(addr >> 8));
	W25Q_ReadWriteByte((uint8_t)addr);
	W25Q_CS_Set();
	W25Q_Wait();
}

void W25Q_EraseChip(void)
{
	W25Q_EnableWrite();
	W25Q_Wait();
	W25Q_CS_Clr();
	W25Q_ReadWriteByte(W25X_ChipErase);
	W25Q_CS_Set();
	W25Q_Wait();	
}

void W25Q_EnableWrite(void)
{
	W25Q_CS_Clr();
	W25Q_ReadWriteByte(W25X_WriteEnable);
	W25Q_CS_Set();
}

void W25Q_DisableWrite(void)
{
	W25Q_CS_Clr();
	W25Q_ReadWriteByte(W25X_WriteDisable);
	W25Q_CS_Set();
}

void W25Q_Wait(void)
{
	while((W25Q_ReadReg(W25X_ReadStatusReg) & 0x01)==0x01);
}

uint8_t W25Q_ReadWriteByte(uint8_t dat)
{
	while (spi_i2s_flag_get(SPI2, SPI_I2S_TDBE_FLAG) == RESET);
	spi_i2s_data_transmit(SPI2, dat);
	
	while (spi_i2s_flag_get(SPI2, SPI_I2S_RDBF_FLAG) == RESET);
	return spi_i2s_data_receive(SPI2);
}

uint16_t W25Q_ReadReg(uint8_t addr)
{
	uint16_t temp = 0;
	W25Q_CS_Clr();
	W25Q_ReadWriteByte(addr);
	W25Q_ReadWriteByte(0x00);
	W25Q_ReadWriteByte(0x00);
	W25Q_ReadWriteByte(0x00);
	
	temp = W25Q_ReadWriteByte(0xFF) << 8;
	temp |= W25Q_ReadWriteByte(0xFF);
	//SEGGER_RTT_SetTerminal(1);
	//SEGGER_RTT_printf(0, "SPI2 25qxx reg 0x%02x=0x%04x\r\n", addr, temp);
	//SEGGER_RTT_SetTerminal(0);
	W25Q_CS_Set();
	return temp;
}

void W25Q_Read(uint8_t* pBuffer, uint32_t addr, uint16_t size)
{
	uint16_t i;
	W25Q_CS_Clr();
	W25Q_ReadWriteByte(W25X_ReadData);
	W25Q_ReadWriteByte((uint8_t)(addr >> 16));
	W25Q_ReadWriteByte((uint8_t)(addr >> 8));
	W25Q_ReadWriteByte((uint8_t)addr);
	for(i=0; i<size; i++)
	{
		pBuffer[i] = W25Q_ReadWriteByte(0xFF);
	}
	W25Q_CS_Set();
}