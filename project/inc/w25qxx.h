#ifndef __W25QXX_H
#define __W25QXX_H

#include "at32f421_wk_config.h"

#define W25Q_CS_Clr()   gpio_bits_reset(GPIOA, GPIO_PINS_15) //PA15
#define W25Q_CS_Set()   gpio_bits_set(GPIOA, GPIO_PINS_15)

#define W25Q80 	0XEF13 	
#define W25Q16 	0XEF14
#define W25Q32 	0XEF15
#define W25Q64 	0XEF16
#define W25Q128	0XEF17

#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 

void W25Q_SPI2_Init(void);

void W25Q_Write(uint8_t* pBuffer, uint32_t addr, uint16_t size);
void W25Q_NoCheckWrite(uint8_t* pBuffer, uint32_t addr, uint16_t size);
void W25Q_WritePage(uint8_t* pBuffer, uint32_t addr, uint16_t size);
void W25Q_EraseSector(uint32_t addr);
void W25Q_EraseChip(void);
void W25Q_EnableWrite(void);
void W25Q_DisableWrite(void);
void W25Q_Wait(void);
uint8_t W25Q_ReadWriteByte(uint8_t dat);
uint16_t W25Q_ReadReg(uint8_t addr);
void W25Q_Read(uint8_t* pBuffer, uint32_t addr, uint16_t size);

#endif