#ifndef __LCD_MENU_H
#define __LCD_MENU_H

#include "at32f421_wk_config.h"
#include "Queue.h"

void menu_main_display(ina226_info_struct *ina226_info);
void menu_statistics_display(ina226_info_struct *ina226_info);
void menu_2nd_display(void);
void LCD_ChartPrint(char flag, char unit, struct Queue* queue);

#endif