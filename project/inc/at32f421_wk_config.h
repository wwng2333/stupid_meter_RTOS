/* add user code begin Header */
/**
  **************************************************************************
  * @file     at32f421_wk_config.h
  * @brief    header file of work bench config
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */
/* add user code end Header */

/* define to prevent recursive inclusion -----------------------------------*/
#ifndef __AT32F421_WK_CONFIG_H
#define __AT32F421_WK_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* includes -----------------------------------------------------------------------*/
#include "at32f421.h"

/* private includes -------------------------------------------------------------*/
/* add user code begin private includes */
#include "SEGGER_RTT.h"
#include "delay.h"
#include <stdio.h>
/* add user code end private includes */

/* exported types -------------------------------------------------------------*/
/* add user code begin exported types */
typedef struct key_state_struct
{
	uint8_t key_pressed_time;
	uint16_t key_hold_time;
	uint8_t released;
} key_state_struct;

typedef struct ina226_info_struct
{
	float Voltage;
	float Current;
	float Power;
} ina226_info_struct;

typedef struct ADC_result_struct
{
	uint16_t result[2];
	float temp;
	float vcc;
} ADC_result_struct;

/* add user code end exported types */

/* exported constants --------------------------------------------------------*/
/* add user code begin exported constants */
#define KEY_PIN_Pin GPIO_PINS_0
#define KEY_PIN_GPIO_Port GPIOB

#define LCD_KEY_UPDATE_FLAG (1U << 0)
#define LCD_MAIN_UPDATE_FLAG (1U << 1)

#define __USE_SPI_DMA
/* add user code end exported constants */

/* exported macro ------------------------------------------------------------*/
/* add user code begin exported macro */

/* add user code end exported macro */

/* exported functions ------------------------------------------------------- */
/* system clock config. */
void wk_system_clock_config(void);

/* config periph clock. */
void wk_periph_clock_config(void);

/* nvic config. */
void wk_nvic_config(void);

void wk_gpio_config(void);
void wk_dma1_channel1_init(void);
void wk_adc1_init(void);
void ADC_timer_cb(void);
/* add user code begin exported functions */

/* add user code end exported functions */

#ifdef __cplusplus
}
#endif

#endif
