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
#include <stdbool.h>
/* private includes ----------------------------------------------------------*/
/* add user code begin private includes */

/* add user code end private includes */

/* private typedef -----------------------------------------------------------*/
/* add user code begin private typedef */

/* add user code end private typedef */

/* private define ------------------------------------------------------------*/
/* add user code begin private define */
//#define __ENABLE_EventRecorder
/* add user code end private define */

/* private macro -------------------------------------------------------------*/
/* add user code begin private macro */

/* add user code end private macro */

/* private variables ---------------------------------------------------------*/
/* add user code begin private variables */

osThreadId_t key_scan_ID, app_main_ID, LCD_Update_ID;
osTimerId_t timer0;
osEventFlagsId_t LCD_Update_flagID;

key_state_struct key_state = {.key_pressed_time = 0, .key_hold_time = 0, .released = 1};
ina226_info_struct ina226_info = {.Voltage = 0.0f, .Current = 0.0f, .Power = 0.0f};
ADC_result_struct ADC_result = {.result = {0}, .temp = 0.0f, .vcc = 0.0f};

struct Queue Voltage_queue = {.front = 0, .rear = 0};
struct Queue Current_queue = {.front = 0, .rear = 0};
struct Queue Power_queue = {.front = 0, .rear = 0};

float mAh = 0.0f;
float mWh = 0.0f;

__IO uint8_t USE_HORIZONTAL = 3;
uint8_t Status = 0;
uint8_t SavedPoint[SIZE] = {0};
char Calc[32] = {0};

bool __SPI_8bit_mode;
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
		.stack_size = 512};

static const osThreadAttr_t ThreadAttr_key_scan =
	{
		.name = "key_scan",
		.priority = (osPriority_t)osPriorityHigh,
		.stack_size = 256};

static const osThreadAttr_t ThreadAttr_LCD_Update =
	{
		.name = "LCD_Update",
		.priority = (osPriority_t)osPriorityNormal2,
		.stack_size = 512};

static const osEventFlagsAttr_t FlagsAttr_LCD_Update_event =
	{
		.name = "LCD_Update_evt"};

static const osTimerAttr_t timerAttr_lcd_cb = {
	.name = "LCD_timer0",
};

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
				if (key_state.key_pressed_time > 0)
				// Key is pressed
				{
					if (key_state.key_hold_time < 200)
					// short press < 200ms
					{
						if (Status < 4)
							Status++;
						else
							Status = 0;
					}
					else 
					// long press > 200ms
					{
						if (USE_HORIZONTAL == 3)
							USE_HORIZONTAL = 2;
						else
							USE_HORIZONTAL = 3;
						LCD_Init_Swap();
					}
					LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);
					osDelay(10);
					osEventFlagsSet(LCD_Update_flagID, LCD_MAIN_UPDATE_FLAG);
					key_state.key_pressed_time = 0;
					key_state.key_hold_time = 0;
				}
			}
		}
		osDelay(50);
	}
}

__NO_RETURN void LCD_Update_thread1(void *arg)
{
	spi_frame_bit_num_set(SPI1, SPI_FRAME_16BIT);
	LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);
	for (;;)
	{
		osEventFlagsWait(LCD_Update_flagID, LCD_MAIN_UPDATE_FLAG, osFlagsWaitAny, osWaitForever);
		osKernelLock();
		ina226_info.Voltage = INA226_Read_Voltage();
		ina226_info.Current = INA226_Read_Current();
		ina226_info.Power = ina226_info.Voltage * ina226_info.Current;
		osKernelUnlock();
		osDelay(50);
		enqueue(&Voltage_queue, ina226_info.Voltage);
		enqueue(&Current_queue, ina226_info.Current);
		enqueue(&Power_queue, ina226_info.Power);
		mAh += 0.5 * ina226_info.Current / 3.6;
		mWh += 0.5 * ina226_info.Power / 3.6;
		switch (Status)
		{
		case 0:
			LCD_DrawLine(88, 2, 88, 78, WHITE);
			LCD_DrawLine(89, 2, 89, 78, WHITE);
			if (ina226_info.Voltage < 10)
				sprintf(Calc, "%.3fV", ina226_info.Voltage);
			else
				sprintf(Calc, "%.2fV", ina226_info.Voltage);
			LCD_ShowString2416(0, 2, Calc, LIGHTBLUE, BLACK);

			sprintf(Calc, "%.3fA", ina226_info.Current);
			LCD_ShowString2416(0, 29, Calc, BLUE, BLACK);
#ifdef __Crazy_DEBUG
			SEGGER_RTT_printf(0, "%s\r\n", Calc);
#endif

			if (ina226_info.Power < 10)
				sprintf(Calc, "%.3fW", ina226_info.Power);
			else if (ina226_info.Power < 100)
				sprintf(Calc, "%.2fW", ina226_info.Power);
			else
				sprintf(Calc, "%.1fW", ina226_info.Power);
			LCD_ShowString2416(0, 56, Calc, GBLUE, BLACK);
#ifdef __Crazy_DEBUG
			SEGGER_RTT_printf(0, "%s\r\n", Calc);
#endif

			sprintf(Calc, "MCU:%.1fC", ADC_result.temp);
			LCD_ShowString(96, 2, Calc, GBLUE, BLACK, 12, 0);
			sprintf(Calc, "Vcc:%.2fV", ADC_result.vcc);
			LCD_ShowString(96, 18, Calc, GBLUE, BLACK, 12, 0);
			sprintf(Calc, "%.2fmAh", mAh);
			LCD_ShowString(96, 34, Calc, GBLUE, BLACK, 12, 0);
			if (mWh < 10000)
				sprintf(Calc, "%.2fmWh", mWh);
			else
				sprintf(Calc, "%.1fmWh", mWh);
			LCD_ShowString(96, 50, Calc, GBLUE, BLACK, 12, 0);
			if (USE_HORIZONTAL == 3)
				LCD_ShowString(96, 62, "<------", GBLUE, BLACK, 16, 0);
			else
				LCD_ShowString(96, 62, "------>", GBLUE, BLACK, 16, 0);
			break;

		case 1:
			LCD_ChartPrint('V', 'V', &Voltage_queue);
			break;

		case 2:
			LCD_ChartPrint('A', 'A', &Current_queue);
			break;

		case 3:
			LCD_ChartPrint('P', 'W', &Power_queue);
			break;

		case 4:
			LCD_DrawLine(0, 14, 160, 14, GBLUE);
			sprintf(Calc, "%.1fV %.2fA %.1fW %.1fC   ", ina226_info.Voltage, ina226_info.Current, ina226_info.Power, ADC_result.temp);
			LCD_ShowString(1, 1, Calc, GBLUE, BLACK, 12, 0);
			sprintf(Calc, "Max  Avg  Min");
			LCD_ShowString(18, 14, Calc, GBLUE, BLACK, 16, 1);
			sprintf(Calc, "%c %.2f %.2f %.2f", 'V', Voltage_queue.max, Voltage_queue.avg, Voltage_queue.min);
			LCD_ShowString(1, 30, Calc, GBLUE, BLACK, 16, 0);
			sprintf(Calc, "%c %.2f %.2f %.2f", 'A', Current_queue.max, Current_queue.avg, Current_queue.min);
			LCD_ShowString(1, 46, Calc, GBLUE, BLACK, 16, 0);
			sprintf(Calc, "%c %.2f %.2f %.2f", 'W', Power_queue.max, Power_queue.avg, Power_queue.min);
			LCD_ShowString(1, 62, Calc, GBLUE, BLACK, 16, 0);
			break;
		}
		osEventFlagsClear(LCD_Update_flagID, LCD_MAIN_UPDATE_FLAG);
	}
}

void lcd_timer_cb(void *param)
{
	ADC_timer_cb();
	osEventFlagsSet(LCD_Update_flagID, LCD_MAIN_UPDATE_FLAG);
}

void ADC_timer_cb(void)
{
	adc_ordinary_software_trigger_enable(ADC1, TRUE);
	ADC_result.vcc = ((float)1.2 * 4095) / ADC_result.result[1];
	ADC_result.temp = ((1.26) - (float)ADC_result.result[0] * ADC_result.vcc / 4096) / (-0.00423) + 25;
}

void app_main(void *arg)
{
	INA226_Init();
	LCD_Init();
	__SPI_8bit_mode = 0;
	LCD_Update_flagID = osEventFlagsNew(&FlagsAttr_LCD_Update_event);

	key_scan_ID = osThreadNew(key_scan_thread1, NULL, &ThreadAttr_key_scan);
	LCD_Update_ID = osThreadNew(LCD_Update_thread1, NULL, &ThreadAttr_LCD_Update);

	timer0 = osTimerNew(&lcd_timer_cb, osTimerPeriodic, (void *)0, &timerAttr_lcd_cb);
	osTimerStart(timer0, 500);
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
	// delay_init();
#ifdef __ENABLE_EventRecorder
	EventRecorderInitialize(EventRecordAll, 10U);
	EventRecorderStart();
#endif
	/* nvic config. */
	wk_nvic_config();
	wk_gpio_config();

	/* adc config. */
	wk_dma1_channel1_init();
	wk_adc1_init();
	wk_dma_channel_config(DMA1_CHANNEL1, (uint32_t)&ADC1->odt, (uint32_t)ADC_result.result, 2);
	dma_channel_enable(DMA1_CHANNEL1, TRUE);
	adc_ordinary_software_trigger_enable(ADC1, TRUE);

	/* config periph clock. */
	wk_periph_clock_config();
	I2C_Soft_Init();

	/* config LCD screen. */
	LCD_SPI1_init();

	/* add user code begin 2 */
	osKernelInitialize();
	app_main_ID = osThreadNew(app_main, NULL, &ThreadAttr_app_main);
	while (osKernelGetState() != osKernelReady) {};
	osKernelStart();
	/* add user code end 2 */

	while (1)
	{
		/* add user code begin 3 */

		/* add user code end 3 */
	}
}
