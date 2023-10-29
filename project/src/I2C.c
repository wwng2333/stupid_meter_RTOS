#include "at32f421_wk_config.h"
#include "cmsis_os2.h"
#include "I2C.h"

extern ina226_info_struct ina226_info;

//#define __Crazy_DEBUG
#pragma clang optimize off
void I2C_Delay(void)
{
   __IO uint8_t i;
  for (i = 0; i < 100; i++)
	{
		;
	}
}
#pragma clang optimize on

void INA226_Update(void)
{
	INA226_Read_Voltage();
	osDelay(1);
	INA226_Read_Current();
	osDelay(1);
	ina226_info.Power = ina226_info.Voltage * ina226_info.Current;
}

void INA226_Read_Voltage(void)
{
	uint16_t temp = 0;
	temp = I2C_Read_2Byte(0x02);
	ina226_info.Voltage = 1.25 * (float)I2C_Read_2Byte(0x02) / 1000;
	#ifdef __Crazy_DEBUG
	SEGGER_RTT_SetTerminal(1);
	SEGGER_RTT_printf(0, "read INA226 0x02=%d\r\n", temp);
	SEGGER_RTT_SetTerminal(0);
	#endif
}

void INA226_Read_Current(void)
{
	uint16_t temp = 0;
	temp = I2C_Read_2Byte(0x04);
	if(temp&0x8000) 
	{
		temp = ~(temp - 1);
		ina226_info.Direction = 1;
	}
	else
	{
		ina226_info.Direction = 0;
	}
	ina226_info.Current = (float)temp * 0.0002;
	#ifdef __Crazy_DEBUG
	SEGGER_RTT_SetTerminal(1);
	SEGGER_RTT_printf(0, "read INA226 0x04=%d\r\n", temp);
	SEGGER_RTT_SetTerminal(0);
	#endif
}

//float INA226_Read_Power(void)
//{
//	return (float)I2C_Read_2Byte(0x03) * 0.02 * 25;
//}

void I2C_Soft_Init(void)
{
	/* enable gpiof periph clock */
  crm_periph_clock_enable(CRM_GPIOF_PERIPH_CLOCK, TRUE);
	
	gpio_init_type gpio_init_struct;
	gpio_default_para_init(&gpio_init_struct);
	
	/* gpio output config */
	gpio_bits_write(SDA_PORT, SDA_PIN | SCL_PIN, TRUE); 
	
	gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_OPEN_DRAIN;
  gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
  gpio_init_struct.gpio_pins = SDA_PIN | SCL_PIN;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(SDA_PORT, &gpio_init_struct);
}

/**
  * @brief Read INA226 ID, Set Configuration 
	* Register and Calibration Register
  * @param  none
  * @retval none
  */
void INA226_Init(void)
{
	uint16_t id = 0;
	do {
		id = I2C_Read_2Byte(0xFE);
		#ifdef __Crazy_DEBUG
		SEGGER_RTT_SetTerminal(1);
		SEGGER_RTT_printf(0, "read INA226 0xFE=0x%x\r\n", id);
		#endif
		osDelay(100);
	} while(id != 0x5449);
	I2C_Write_2Byte(0x00, 0x45FF); // Configuration Register
	I2C_Write_2Byte(0x05, 0x1400); // Calibration Register, 5120, 0.1mA
	//LSB=0.0002mA,R=0.005R Cal=0.00512/(0.0002*0.005)=5120=0x1400
	#ifdef __Crazy_DEBUG
	SEGGER_RTT_printf(0, "set INA226 0x05=0x%x\r\n", 0x1400);
	SEGGER_RTT_SetTerminal(0);
	#endif
}

uint16_t I2C_Read_2Byte(uint8_t addr)
{
	uint16_t dat = 0;
	I2C_Start();
	I2C_SendData(INA226);
	I2C_RecvACK();
	I2C_SendData(addr);
	I2C_RecvACK();
	I2C_Start();
	I2C_SendData(INA226 + 1);
	I2C_RecvACK();
	dat = I2C_RecvData();
	dat <<= 8;
	I2C_SendACK();
	dat |= I2C_RecvData();
	I2C_SendNAK();
	I2C_Stop();

	return dat;
}

uint8_t I2C_ReadByte(uint8_t addr)
{
	uint8_t dat = 0;
	I2C_Start();
	I2C_SendData(INA226);
	I2C_RecvACK();
	I2C_SendData(addr);
	I2C_RecvACK();

	I2C_Delay();

	I2C_Start();
	I2C_SendData(INA226 + 1);
	I2C_RecvACK();
	dat = I2C_RecvData();
	I2C_SendNAK();
	I2C_Stop();
	return dat;
}

void I2C_Write_2Byte(uint8_t addr, uint16_t dat)
{
	I2C_Start();
	I2C_SendData(INA226);
	I2C_RecvACK();
	I2C_SendData(addr);
	I2C_RecvACK();
	I2C_SendData(dat >> 8);
	I2C_RecvACK();
	I2C_SendData(dat);
	I2C_RecvACK();
	I2C_Stop();
}

void I2C_WriteByte(uint8_t addr, uint8_t dat)
{
	I2C_Start();
	I2C_SendData(INA226);
	I2C_RecvACK();
	I2C_SendData(addr);
	I2C_RecvACK();
	I2C_SendData(dat);
	I2C_RecvACK();
	I2C_Stop();
}

void I2C_Start()
{
	I2C_SDA_Set();
	I2C_SCL_Set();
	I2C_Delay();
	I2C_SDA_Clr();
	I2C_Delay();
	I2C_SCL_Clr();
}

void I2C_SendData(uint8_t dat)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		I2C_SCL_Clr();
		I2C_Delay();
		if (dat & 0x80)
			I2C_SDA_Set();
		else
			I2C_SDA_Clr();
		dat <<= 1;
		I2C_SCL_Set();
		I2C_Delay();
	}
	I2C_SCL_Clr();
}

void I2C_SendACK()
{
	I2C_SCL_Clr();
	I2C_Delay();
	I2C_SDA_Clr();
	I2C_SCL_Set();
	I2C_Delay();
	I2C_SCL_Clr();
	I2C_SDA_Set();
	I2C_Delay();
}

void I2C_RecvACK()
{
	I2C_SCL_Clr();
	I2C_Delay();
	I2C_SDA_Set();
	I2C_SCL_Set();
	I2C_Delay();
	I2C_SCL_Clr();
	I2C_Delay();
}

uint8_t I2C_RecvData()
{
	uint8_t dat, i;
	for (i = 0; i < 8; i++)
	{
		dat <<= 1;
		I2C_SCL_Clr();
		I2C_Delay();
		I2C_SCL_Set();
		I2C_Delay();
		if (Read_SDA_Pin == SET)
		{
			dat += 1;
		}
	}
	return dat;
}

void I2C_SendNAK()
{
	I2C_SCL_Clr();
	I2C_Delay();
	I2C_SDA_Set();
	I2C_SCL_Set();
	I2C_Delay();
	I2C_SCL_Clr();
	I2C_Delay();
}

void I2C_Stop()
{
	I2C_SDA_Clr();
	I2C_SCL_Set();
	I2C_Delay();
	I2C_SDA_Set();
	I2C_Delay();
}