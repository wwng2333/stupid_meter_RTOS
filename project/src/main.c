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
//#define __ENABLE_EventRecorder
//#define __ENABLE_EasyFlash
#define __ENABLE_TFDB
/* add user code end Header */

/* Includes ------------------------------------------------------------------*/
#include "at32f421_wk_config.h"
#include "I2C.h"
#include "cmsis_os2.h"
#include "RTE_Components.h"
#include "Queue.h"
#include "lcd.h"
#include "lcd_init.h"
#include <stdbool.h>
#ifdef __ENABLE_EventRecorder
#include "EventRecorder.h"
#endif
#ifdef __ENABLE_EasyFlash
#include "sfud.h"
#include <easyflash.h>
#endif
#ifdef __ENABLE_TFDB
#include "tinyflashdb.h"
#include "w25qxx.h"
#endif
/* private includes ----------------------------------------------------------*/
/* add user code begin private includes */

/* add user code end private includes */

/* private typedef -----------------------------------------------------------*/
/* add user code begin private typedef */

/* add user code end private typedef */

/* private define ------------------------------------------------------------*/
/* add user code begin private define */
#ifdef __ENABLE_TFDB
const tfdb_index_t test_index = {
    .end_byte = 0x00,
    .flash_addr = 0x4000,
    .flash_size = 256,
    .value_length = 1,
};
tfdb_addr_t addr = 0;
uint8_t test_buf[TFDB_ALIGNED_RW_BUFFER_SIZE(2,1)];
uint16_t test_value;

void tfdb_test(void)
{
	TFDB_Err_Code result;

	result = tfdb_get(&test_index, test_buf, &addr, &test_value);
	if(result == TFDB_NO_ERR)
	{
			printf("get ok, addr:%x, value:%x\n", addr, test_value);
	}	
	
	test_value++;
	
	result = tfdb_set(&test_index, test_buf, &addr, &test_value);
	if(result == TFDB_NO_ERR)
	{
			printf("set ok, addr:%x\n", addr);
	}

}

#endif
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
ina226_info_struct ina226_info = {.Voltage = 0.0f, .Current = 0.0f, .Power = 0.0f, .Direction = 0};
ADC_result_struct ADC_result = {.result = {0}, .temp = 0.0f, .vcc = 0.0f};
menu_state_enum menu_state = menu_default;
screen_direction_enum screen_direction = SCREEN_HORIZONTAL_REVERSED;

struct Queue Voltage_queue = {.front = 0, .rear = 0};
struct Queue Current_queue = {.front = 0, .rear = 0};
struct Queue Power_queue = {.front = 0, .rear = 0};

float mAh = 0.0f;
float mWh = 0.0f;
float time_past = 0.0f;

uint8_t Status = 0;
uint8_t SavedPoint[SIZE] = {0};
uint32_t i2c_last_tick;
char sprintf_buf[32] = {0};

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
		.stack_size = 2048};

static const osThreadAttr_t ThreadAttr_key_scan =
	{
		.name = "key_scan",
		.priority = (osPriority_t)osPriorityRealtime,
		.stack_size = 256};

static const osThreadAttr_t ThreadAttr_LCD_Update =
	{
		.name = "LCD_Update",
		.priority = (osPriority_t)osPriorityNormal,
		.stack_size = 768};

static const osEventFlagsAttr_t FlagsAttr_LCD_Update_event =
	{
		.name = "LCD_Update_evt"};

static const osTimerAttr_t timerAttr_lcd_cb = {
	.name = "LCD_timer0",
};

#ifdef __ENABLE_EasyFlash
/**
 * Env demo.
 */
static void test_env(void) 
{
	uint32_t i_boot_times = NULL;
	char *c_old_boot_times, c_new_boot_times[11] = {0};

	/* get the boot count number from Env */
	c_old_boot_times = ef_get_env("boot_times");
	//assert_param(c_old_boot_times);
	i_boot_times = atol(c_old_boot_times);
	/* boot count +1 */
	i_boot_times ++;
	printf("The system now boot %d times\n\r", i_boot_times);
	/* interger to string */
	sprintf(c_new_boot_times,"%u", i_boot_times);
	/* set and store the boot count number to Env */
	ef_set_env("boot_times", c_new_boot_times);
	ef_save_env();
}
#endif

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
						if (menu_state != menu_2nd_menu)
							menu_state++;
						else
							menu_state = menu_default;
					}
					else
					// long press > 200ms
					{
						if (screen_direction == SCREEN_HORIZONTAL_REVERSED)
							screen_direction = SCREEN_HORIZONTAL;
						else
							screen_direction = SCREEN_HORIZONTAL_REVERSED;
						LCD_Init_Swap();
					}
					LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);
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
	i2c_last_tick = osKernelGetTickCount();
	for (;;)
	{
		osEventFlagsWait(LCD_Update_flagID, LCD_MAIN_UPDATE_FLAG, osFlagsWaitAny, osWaitForever);
		osKernelLock();
		INA226_Update();
		osKernelUnlock();
		time_past = (float)(osKernelGetTickCount() - i2c_last_tick) / 3600;
		i2c_last_tick = osKernelGetTickCount();
		osDelay(50);
		enqueue(&Voltage_queue, ina226_info.Voltage);
		enqueue(&Current_queue, ina226_info.Current);
		enqueue(&Power_queue, ina226_info.Power);
		mAh += time_past * ina226_info.Current;
		mWh += time_past * ina226_info.Power;
		osKernelLock();
		switch (menu_state)
		{
		case menu_default:
			menu_main_display();
			break;

		case menu_voltage_chart:
			LCD_ChartPrint('V', 'V', &Voltage_queue);
			break;

		case menu_current_chart:
			LCD_ChartPrint('A', 'A', &Current_queue);
			break;

		case menu_power_chart:
			LCD_ChartPrint('P', 'W', &Power_queue);
			break;

		case menu_statistics:
			menu_statistics_display();
			break;
		
		case menu_2nd_menu:
			menu_2nd_display();
			break;
		}
		osKernelUnlock();
		osEventFlagsClear(LCD_Update_flagID, LCD_MAIN_UPDATE_FLAG);
	}
}

void menu_main_display(void)
{
	LCD_DrawLine(88, 2, 88, 78, WHITE);
	LCD_DrawLine(89, 2, 89, 78, WHITE);
	if (ina226_info.Voltage < 10)
		sprintf(sprintf_buf, "%.3fV", ina226_info.Voltage);
	else
		sprintf(sprintf_buf, "%.2fV", ina226_info.Voltage);
	LCD_ShowString2416(0, 2, sprintf_buf, LIGHTBLUE, BLACK);

	sprintf(sprintf_buf, "%.3fA", ina226_info.Current);
	LCD_ShowString2416(0, 29, sprintf_buf, BLUE, BLACK);
#ifdef __Crazy_DEBUG
	SEGGER_RTT_printf(0, "%s\r\n", sprintf_buf);
#endif

	if (ina226_info.Power < 10)
		sprintf(sprintf_buf, "%.3fW", ina226_info.Power);
	else if (ina226_info.Power < 100)
		sprintf(sprintf_buf, "%.2fW", ina226_info.Power);
	else
		sprintf(sprintf_buf, "%.1fW", ina226_info.Power);
	LCD_ShowString2416(0, 56, sprintf_buf, GBLUE, BLACK);
#ifdef __Crazy_DEBUG
	SEGGER_RTT_printf(0, "%s\r\n", sprintf_buf);
#endif

	sprintf(sprintf_buf, "MCU:%.1fC", ADC_result.temp);
	LCD_ShowString(96, 2, sprintf_buf, GBLUE, BLACK, 12, 0);
	sprintf(sprintf_buf, "Vcc:%.2fV", ADC_result.vcc);
	LCD_ShowString(96, 18, sprintf_buf, GBLUE, BLACK, 12, 0);
	sprintf(sprintf_buf, "%.2fmAh", mAh);
	LCD_ShowString(96, 34, sprintf_buf, GBLUE, BLACK, 12, 0);

	if (mWh < 10000)
		sprintf(sprintf_buf, "%.2fmWh", mWh);
	else
		sprintf(sprintf_buf, "%.1fmWh", mWh);
	LCD_ShowString(96, 50, sprintf_buf, GBLUE, BLACK, 12, 0);

	if ((screen_direction == SCREEN_HORIZONTAL_REVERSED && ina226_info.Direction) || screen_direction == SCREEN_HORIZONTAL)
	{
		LCD_ShowString(96, 62, "<------", GBLUE, BLACK, 16, 0);
	} 
	else if((screen_direction == SCREEN_HORIZONTAL && ina226_info.Direction) || screen_direction == SCREEN_HORIZONTAL_REVERSED)
	{
		LCD_ShowString(96, 62, "------>", GBLUE, BLACK, 16, 0);
	}
	else
	{
		LCD_ShowString(96, 62, "-------", GBLUE, BLACK, 16, 0);
	}
}

void menu_statistics_display(void)
{
	LCD_DrawLine(0, 14, 160, 14, GBLUE);
	sprintf(sprintf_buf, "%.1fV %.2fA %.1fW %.1fC   ", ina226_info.Voltage, ina226_info.Current, ina226_info.Power, ADC_result.temp);
	LCD_ShowString(1, 1, sprintf_buf, GBLUE, BLACK, 12, 0);
	sprintf(sprintf_buf, "Max A. M. %.1fs", (float)osKernelGetTickCount()/1000);
	LCD_ShowString(1, 15, sprintf_buf, GBLUE, BLACK, 16, 0);
	sprintf(sprintf_buf, "%c %.2f %.2f %.2f", 'V', Voltage_queue.max, Voltage_queue.avg, Voltage_queue.min);
	LCD_ShowString(1, 30, sprintf_buf, GBLUE, BLACK, 16, 0);
	sprintf(sprintf_buf, "%c %.2f %.2f %.2f", 'A', Current_queue.max, Current_queue.avg, Current_queue.min);
	LCD_ShowString(1, 46, sprintf_buf, GBLUE, BLACK, 16, 0);
	sprintf(sprintf_buf, "%c %.2f %.2f %.2f", 'W', Power_queue.max, Power_queue.avg, Power_queue.min);
	LCD_ShowString(1, 62, sprintf_buf, GBLUE, BLACK, 16, 0);
}

void menu_2nd_display(void)
{
	char infobuf[16];
	osVersion_t osv;
	osStatus_t status;
	status = osKernelGetInfo(&osv, infobuf, sizeof(infobuf));

	LCD_DrawLine(0, 14, 160, 14, GBLUE);
	sprintf(sprintf_buf, "Crazy USB meter by wwng");
	LCD_ShowString(1, 1, sprintf_buf, GBLUE, BLACK, 12, 0);
	sprintf(sprintf_buf, "MCU: AT32F421G8U7 %d", __AT32F421_LIBRARY_VERSION);
	LCD_ShowString(1, 15, sprintf_buf, GBLUE, BLACK, 12, 0);
	sprintf(sprintf_buf, "INA226 ID: 0x%02x 0x%02x", I2C_Read_2Byte(0xFE), I2C_Read_2Byte(0xFF));
	LCD_ShowString(1, 27, sprintf_buf, GBLUE, BLACK, 12, 0);
	sprintf(sprintf_buf, "%s kernel %d", infobuf, osv.kernel);
	LCD_ShowString(1, 39, sprintf_buf, GBLUE, BLACK, 12, 0);
	sprintf(sprintf_buf, "key_scan: %d/%d bytes.", osThreadGetStackSpace(key_scan_ID), osThreadGetStackSize(key_scan_ID));
	LCD_ShowString(1, 51, sprintf_buf, GBLUE, BLACK, 12, 0);
	sprintf(sprintf_buf, "LCD_Update: %d/%d bytes.", osThreadGetStackSpace(LCD_Update_ID), osThreadGetStackSize(LCD_Update_ID));
	LCD_ShowString(1, 63, sprintf_buf, GBLUE, BLACK, 12, 0);
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
#ifdef __ENABLE_EasyFlash
	if (easyflash_init() == EF_NO_ERR) {
			test_env();
	} 
#endif

#ifdef __ENABLE_TFDB
	tfdb_test();
#endif
	
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
	W25Q_SPI2_Init();

	/* add user code begin 2 */
	osKernelInitialize();
	app_main_ID = osThreadNew(app_main, NULL, &ThreadAttr_app_main);
	while (osKernelGetState() != osKernelReady)
	{
	};
	osKernelStart();
	/* add user code end 2 */

	while (1)
	{
		/* add user code begin 3 */

		/* add user code end 3 */
	}
}
