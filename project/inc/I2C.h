#ifndef __I2C_H__
#define __I2C_H__

#define INA226 0x80

#include "at32f421_wk_config.h"

#define SDA_PIN GPIO_PINS_0
#define SDA_PORT GPIOF
#define SCL_PIN GPIO_PINS_1
#define SCL_PORT GPIOF

#define I2C_SCL_Set() gpio_bits_set(SCL_PORT, SCL_PIN) //PF1
#define I2C_SCL_Clr() gpio_bits_reset(SCL_PORT, SCL_PIN)

#define I2C_SDA_Set() gpio_bits_set(SDA_PORT, SDA_PIN) //PF0
#define I2C_SDA_Clr() gpio_bits_reset(SDA_PORT, SDA_PIN)

#define Read_SDA_Pin gpio_input_data_bit_read(SDA_PORT, SDA_PIN)

//void INA226_Update(ina226_info_struct *ina226_info);

void I2C_Delay(void);
void INA226_Init(void);
float INA226_Read_Voltage(void);
float INA226_Read_Current(void);
//float INA226_Read_Power(void);

uint16_t I2C_Read_2Byte(uint8_t addr);
void I2C_Write_2Byte(uint8_t addr, uint16_t dat);
uint8_t I2C_ReadByte(uint8_t addr);
void I2C_WriteByte(uint8_t addr, uint8_t dat);
uint16_t I2C_Read_12Bit(uint8_t addr);
uint16_t I2C_Read_13Bit(uint8_t addr);
uint32_t I2C_Read_32Bit(uint8_t addr);

void I2C_Start();
void I2C_SendData(uint8_t dat);
void I2C_RecvACK();
void I2C_SendACK();
uint8_t I2C_RecvData();
void I2C_SendNAK();
void I2C_Stop();

#endif