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
uint16_t test1 = 0;
uint16_t test2 = 0;
/* add user code end private variables */

/* private function prototypes --------------------------------------------*/
/* add user code begin function prototypes */

/* add user code end function prototypes */

/* private user code ---------------------------------------------------------*/
/* add user code begin 0 */

__NO_RETURN void Thread_1(void *arg)
{
	__IO uint8_t test3[256] = {0};
	for (;;)
	{
		test1++;
		test3[test1] = test1;
		osDelay(50);
	}
}

__NO_RETURN void Thread_2(void *arg)
{
	for (;;)
	{
		test2++;
		osDelay(500);
	}
}

const osThreadAttr_t Thread_1_attr = { .stack_size = 512 };
const osThreadAttr_t Thread_2_attr = { .stack_size = 128 };

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

  /* config periph clock. */
  wk_periph_clock_config();
	delay_init();
	I2C_Soft_Init();
	INA226_Init();

  /* nvic config. */
  wk_nvic_config();

  /* add user code begin 2 */
	osKernelInitialize();
	
	osThreadNew(Thread_1, NULL, &Thread_1_attr);
	osThreadNew(Thread_2, NULL, &Thread_2_attr);
	
	osKernelStart(); 
  /* add user code end 2 */

  while(1)
  {
    /* add user code begin 3 */

    /* add user code end 3 */
  }
}
