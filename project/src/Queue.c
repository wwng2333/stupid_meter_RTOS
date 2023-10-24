#include "at32f421_wk_config.h"
#include "Queue.h"
#include "math.h"
#include "lcd.h"

extern uint8_t SavedPoint[SIZE];

void Countqueue(struct Queue* queue)
{
	float max = 0.0f, min = 10000.0f, avg = 0.0f;
	int i = queue->front;
	while (i != queue->rear) {
		if(queue->arr[i] > max) max = queue->arr[i];
		else if(queue->arr[i] < min) min = queue->arr[i];
		avg += queue->arr[i];
		i = (i + 1) % SIZE;
	}
	queue->max = max;
	queue->min = min;
	queue->avg = avg / SIZE;
}

void enqueue(struct Queue* queue, float item) {
	#ifdef __Crazy_DEBUG
	SEGGER_RTT_SetTerminal(3);
	SEGGER_RTT_printf(0, "enqueue=%f\n", item);
	SEGGER_RTT_SetTerminal(0);
	#endif
	Countqueue(queue);
	if ((queue->rear + 1) % SIZE == queue->front) {
		queue->front = (queue->front + 1) % SIZE;
	}
	queue->arr[queue->rear] = item;
	queue->rear = (queue->rear + 1) % SIZE;
}

//int dequeue(struct Queue* queue) {
//    if (queue->front == queue->rear) {
//        SEGGER_RTT_printf(0, "Queue is empty\n");
//        return -1;
//    }
//    int item = queue->arr[queue->front];
//    queue->front = (queue->front + 1) % SIZE;
//    return item;
//}

void ClearPrint(void)
{
	for(int i=0;i<SIZE;i++)
	{
		LCD_DrawPoint(i, SavedPoint[i], BLACK);
	}
}

void printQueue(struct Queue* queue) {
    int i = queue->front;
		uint8_t x = 0;
		uint8_t y0,y1 = 0;
    while (i != queue->rear) {
			y0 = (uint8_t) (80-(queue->arr[i] / queue->max * 66));
			//y1 = (uint8_t) (80-(queue->arr[i+1] / queue->max * 66));
			SavedPoint[x] = y0;
			//SavedPoint[x+1] = y1;
			LCD_DrawPoint(x, y0, WHITE);
			//LCD_DrawLine(x,y0,x+1,y1, WHITE);
			#ifdef __Crazy_DEBUG
			SEGGER_RTT_SetTerminal(3);
			SEGGER_RTT_printf(0, "max=%f ", queue->max);
			SEGGER_RTT_printf(0, "arr[%d]=%f", i, queue->arr[i]);
			SEGGER_RTT_printf(0, "x=%d, y=%d\r\n", x, y0);
			SEGGER_RTT_SetTerminal(0);
			#endif
			i = (i + 1) % SIZE;
			x++;
    }
    //SEGGER_RTT_printf(0, "\n");
}