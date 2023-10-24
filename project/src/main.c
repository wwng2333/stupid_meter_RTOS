/* add user code begin Header */
/**
  **************************************************************************
  * @file     main.c
  * @brief    main program
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

/* Includes ------------------------------------------------------------------*/
#include "at32f421_wk_config.h"
#include "I2C.h"
#include "cmsis_os2.h"
#include "RTE_Components.h"
#include "EventRecorder.h"
#include "Queue.h"
#include "lcd.h"
#include "lcd_init.h"
/* private includes ----------------------------------------------------------*/
/* add user code begin private includes */

/* add user code end private includes */

/* private typedef -----------------------------------------------------------*/
/* add user code begin private typedef */

/* add user code end private typedef */

/* private define ------------------------------------------------------------*/
/* add user code begin private define */

/* add user code end private define */

/* private macro -------------------------------------------------------------*/
/* add user code begin private macro */

/* add user code end private macro */

/* private variables ---------------------------------------------------------*/
/* add user code begin private variables */

osThreadId_t key_scan_ID, app_main_ID, i2c_read_ID, LCD_Update_ID, ADC_Update_ID;

key_state_struct key_state = {.key_pressed_time = 0, .key_hold_time = 0, .released = 1};
ina226_info_struct ina226_info = {.Voltage = 0.0f, .Current = 0.0f, .Power = 0.0f};
ADC_result_struct ADC_result = {.result = {0}, .temp = 0.0f, .vcc = 0.0f};

__IO uint8_t USE_HORIZONTAL = 3;
uint8_t SavedPoint[SIZE] = {0};

/* add user code end private variables */
/* private function prototypes --------------------------------------------*/
/* add user code begin function prototypes */

/* add user code end function prototypes */

/* private user code ---------------------------------------------------------*/
/* add user code begin 0 */
static const osThreadAttr_t ThreadAttr_app_main =
    {
        .name = "app_main",
        .priority = (osPriority_t)osPriorityNormal,
        .stack_size = 384};

static const osThreadAttr_t ThreadAttr_key_scan =
    {
        .name = "key_scan",
        .priority = (osPriority_t)osPriorityNormal,
        .stack_size = 256};
		
static const osThreadAttr_t ThreadAttr_i2c_read =
    {
        .name = "i2c_read",
        .priority = (osPriority_t)osPriorityNormal,
        .stack_size = 384};

static const osThreadAttr_t ThreadAttr_LCD_Update =
    {
        .name = "LCD_Update",
        .priority = (osPriority_t)osPriorityNormal,
        .stack_size = 1024};

static const osThreadAttr_t ThreadAttr_ADC_Update =
    {
        .name = "ADC_Update",
        .priority = (osPriority_t)osPriorityNormal,
        .stack_size = 256};

__NO_RETURN void key_scan_thread1(void *arg)
{
  for (;;)
  {
    if (gpio_input_data_bit_read(KEY_PIN_GPIO_Port, KEY_PIN_Pin) == RESET)
    {
      osDelay(50);
      if (gpio_input_data_bit_read(KEY_PIN_GPIO_Port, KEY_PIN_Pin) == RESET)
      {
        key_state.key_pressed_time++;
        key_state.key_hold_time = 0;
        while (gpio_input_data_bit_read(KEY_PIN_GPIO_Port, KEY_PIN_Pin) == RESET)
        {
          key_state.released = 0;
          key_state.key_hold_time += 50;
          osDelay(50);
        }
        key_state.released = 1;
      }
    }
    osDelay(50);
  }
}

__NO_RETURN void i2c_read_thread1(void *arg)
{
  for (;;)
  {
		ina226_info.Voltage = INA226_Read_Voltage();
		ina226_info.Current = INA226_Read_Current();
		ina226_info.Power = ina226_info.Voltage * ina226_info.Current;
		osDelay(450);
	}
}

__NO_RETURN void LCD_Update_thread1(void *arg)
{
  for (;;)
  {
		LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);
		osDelay(2000);
	}
}

__NO_RETURN void ADC_Update_thread1(void *arg)
{
  for (;;)
  {
		adc_ordinary_software_trigger_enable(ADC1, TRUE);
		ADC_result.vcc = ((float)1.2 * 4095) / ADC_result.result[1];
		ADC_result.temp = ((1.26) - (float)ADC_result.result[0] * ADC_result.vcc / 4096) / (-0.00423) + 25;
		osDelay(500);
	}
}

void app_main(void *arg)
{
	INA226_Init();
	LCD_Init();
	LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);
	
	key_scan_ID = osThreadNew(key_scan_thread1, NULL, &ThreadAttr_key_scan);
	i2c_read_ID = osThreadNew(i2c_read_thread1, NULL, &ThreadAttr_i2c_read);
	LCD_Update_ID = osThreadNew(LCD_Update_thread1, NULL, &ThreadAttr_LCD_Update);
	ADC_Update_ID = osThreadNew(ADC_Update_thread1, NULL, &ThreadAttr_ADC_Update);
}
				
/* add user code end 0 */

/**
  * @brief main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  /* add user code begin 1 */

  /* add user code end 1 */
	
  /* system clock config. */
  wk_system_clock_config();
	//delay_init();
	EventRecorderInitialize(EventRecordAll, 1U);
	EventRecorderStart();
	
  /* nvic config. */
  wk_nvic_config();
	wk_gpio_config();
	
	/* adc config. */
	wk_dma1_channel1_init();
	wk_adc1_init();
	wk_dma_channel_config(DMA1_CHANNEL1, (uint32_t)&ADC1->odt, (uint32_t)ADC_result.result, 2);
	dma_channel_enable(DMA1_CHANNEL1, TRUE);
	
  /* config periph clock. */
  wk_periph_clock_config();
	I2C_Soft_Init();
	
	/* config LCD screen. */
	LCD_SPI1_init();

  /* add user code begin 2 */
	osKernelInitialize();
	app_main_ID = osThreadNew(app_main, NULL, &ThreadAttr_app_main);
  while (osKernelGetState() != osKernelReady);
	osKernelStart();
  /* add user code end 2 */

  while(1)
  {
    /* add user code begin 3 */

    /* add user code end 3 */
  }
}
