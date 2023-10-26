#ifndef __QUEUE_H_
#define __QUEUE_H_

#include "at32f421_wk_config.h"

#define SIZE 130
	
struct Queue {
	float arr[SIZE];
	int front, rear;
	float max, min, avg;
};

void Countqueue(struct Queue* queue);
void enqueue(struct Queue* queue, float item);
//int dequeue(struct Queue* queue);
void printQueue(struct Queue* queue);
void ClearPrint(void);

#endif