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

osThreadId_t key_scan_ID, app_main_ID, i2c_read_ID;

key_state_struct key_state = {.key_pressed_time = 0, .key_hold_time = 0, .released = 1};
ina226_info_struct ina226_info = {.Voltage = 0.0f, .Current = 0.0f, .Power = 0.0f};

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
        .stack_size = 256};

static const osThreadAttr_t ThreadAttr_key_scan =
    {
        .name = "key_scan",
        .priority = (osPriority_t)osPriorityNormal,
        .stack_size = 256};
		
static const osThreadAttr_t ThreadAttr_i2c_read =
    {
        .name = "i2c_read",
        .priority = (osPriority_t)osPriorityNormal,
        .stack_size = 128};


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
		osDelay(500);
	}
}

void app_main(void *arg)
{
	key_scan_ID = osThreadNew(key_scan_thread1, NULL, &ThreadAttr_key_scan);
	i2c_read_ID = osThreadNew(i2c_read_thread1, NULL, &ThreadAttr_i2c_read);
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
	EventRecorderInitialize(EventRecordAll, 1U);
	EventRecorderStart();
  /* add user code end 1 */

  /* system clock config. */
  wk_system_clock_config();

  /* config periph clock. */
  wk_periph_clock_config();
	delay_init();
	I2C_Soft_Init();
	INA226_Init();

  /* nvic config. */
  wk_nvic_config();
	wk_gpio_config();

  /* add user code begin 2 */
	osKernelInitialize();
	app_main_ID = osThreadNew(app_main, NULL, &ThreadAttr_app_main);
  if (osKernelGetState() == osKernelReady)
  {
    osKernelStart();
  }
  /* add user code end 2 */

  while(1)
  {
    /* add user code begin 3 */

    /* add user code end 3 */
  }
}
