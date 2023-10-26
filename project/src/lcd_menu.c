#include "lcd_menu.h"
#include "lcd_init.h"
#include "cmsis_os2.h"
#include "lcd.h"
#include "I2C.h"
#include "Queue.h"

extern bool __SPI_8bit_mode;
extern char sprintf_buf[32];
extern ina226_info_struct ina226_info;
extern ADC_result_struct ADC_result;

//extern struct Queue Voltage_queue, Current_queue, Power_queue;

extern screen_direction_enum screen_direction;
extern float mAh, mWh;

extern osThreadId_t key_scan_ID, LCD_Update_ID;

void menu_main_display(ina226_info_struct *ina226_info)
{
	LCD_DrawLine(88, 2, 88, 78, WHITE);
	LCD_DrawLine(89, 2, 89, 78, WHITE);
	if (ina226_info->Voltage < 10)
		sprintf(sprintf_buf, "%.3fV", ina226_info->Voltage);
	else
		sprintf(sprintf_buf, "%.2fV", ina226_info->Voltage);
	LCD_ShowString2416(0, 2, sprintf_buf, LIGHTBLUE, BLACK);

	sprintf(sprintf_buf, "%.3fA", ina226_info->Current);
	LCD_ShowString2416(0, 29, sprintf_buf, BLUE, BLACK);
#ifdef __Crazy_DEBUG
	SEGGER_RTT_printf(0, "%s\r\n", sprintf_buf);
#endif

	if (ina226_info->Power < 10)
		sprintf(sprintf_buf, "%.3fW", ina226_info->Power);
	else if (ina226_info->Power < 100)
		sprintf(sprintf_buf, "%.2fW", ina226_info->Power);
	else
		sprintf(sprintf_buf, "%.1fW", ina226_info->Power);
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

	if ((screen_direction == SCREEN_HORIZONTAL_REVERSED && ina226_info->Direction) || screen_direction == SCREEN_HORIZONTAL)
	{
		LCD_ShowString(96, 62, "<------", GBLUE, BLACK, 16, 0);
	} 
	else if((screen_direction == SCREEN_HORIZONTAL && ina226_info->Direction) || screen_direction == SCREEN_HORIZONTAL_REVERSED)
	{
		LCD_ShowString(96, 62, "------>", GBLUE, BLACK, 16, 0);
	}
	else
	{
		LCD_ShowString(96, 62, "-------", GBLUE, BLACK, 16, 0);
	}
}

void menu_statistics_display(ina226_info_struct *ina226_info)
{
	LCD_DrawLine(0, 14, 160, 14, GBLUE);
	sprintf(sprintf_buf, "%.1fV %.2fA %.1fW %.1fC   ", ina226_info->Voltage, ina226_info->Current, ina226_info->Power, ADC_result.temp);
	LCD_ShowString(1, 1, sprintf_buf, GBLUE, BLACK, 12, 0);
	sprintf(sprintf_buf, "Max A. M. %.1fs", (float)osKernelGetTickCount()/1000);
	LCD_ShowString(1, 15, sprintf_buf, GBLUE, BLACK, 16, 0);
	sprintf(sprintf_buf, "%c %.2f %.2f %.2f", 'V', ina226_info->Voltage_queue.max, ina226_info->Voltage_queue.avg, ina226_info->Voltage_queue.min);
	LCD_ShowString(1, 30, sprintf_buf, GBLUE, BLACK, 16, 0);
	sprintf(sprintf_buf, "%c %.2f %.2f %.2f", 'A', ina226_info->Current_queue.max, ina226_info->Current_queue.avg, ina226_info->Current_queue.min);
	LCD_ShowString(1, 46, sprintf_buf, GBLUE, BLACK, 16, 0);
	sprintf(sprintf_buf, "%c %.2f %.2f %.2f", 'W', ina226_info->Power_queue.max, ina226_info->Power_queue.avg, ina226_info->Power_queue.min);
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


void LCD_ChartPrint(char flag, char unit, struct Queue* queue)
{
	LCD_DrawLine(0, 14, SIZE, 14, GBLUE);
	LCD_DrawLine(0, 46, SIZE, 46, GBLUE);
	LCD_DrawLine(0, 78, SIZE, 78, GBLUE);
	sprintf(sprintf_buf, "%.1fV %.2fA %.2fW %.1fC   ", ina226_info.Voltage, ina226_info.Current, ina226_info.Power, ADC_result.temp);
	LCD_ShowString(1, 1, sprintf_buf, GBLUE, BLACK, 12, 0);
	LCD_ShowChar(150, 1, flag, GBLUE, BLACK, 12, 0);
	sprintf(sprintf_buf, "0.0%c", unit);
	LCD_ShowString(SIZE+2, 70, sprintf_buf, GBLUE, BLACK, 12, 0);
	if((queue->max)/2 > 10) 
	{
		sprintf(sprintf_buf, "%.0f", (queue->max)/2);
		LCD_ShowString(SIZE+2, 40, sprintf_buf, GBLUE, BLACK, 12, 0);
	}
	else 
	{
		sprintf(sprintf_buf, "%.2f", (queue->max)/2);
		LCD_ShowString(SIZE+2, 40, sprintf_buf, GBLUE, BLACK, 12, 0);
	}
	if(queue->max > 10) 
	{
		sprintf(sprintf_buf, "%.0f", queue->max);
		LCD_ShowString(SIZE+2, 13, sprintf_buf, GBLUE, BLACK, 12, 0);
	}
	else 
	{
		sprintf(sprintf_buf, "%.2f", queue->max);
		LCD_ShowString(SIZE+2, 13, sprintf_buf, GBLUE, BLACK, 12, 0);
	}
	ClearPrint();
	printQueue(queue);
}
