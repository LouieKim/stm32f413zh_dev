/*
 * evon_queue.c
 *
 *  Created on: 2020. 12. 11.
 *      Author: hyeok
 */


#include "main.h"
#include "evon_queue.h"

uint8_t QUEUE_IsEmpty(EVON_QUEUE* queue)
{
	if(queue->front == queue->rear) return 1;
	else return 0;
}

uint8_t QUEUE_IsFull(EVON_QUEUE* queue)
{
	if((queue->front + 1) % queue->size == queue->rear) return 1;
	else return 0;
}

void QUEUE_insert(EVON_QUEUE* queue, uint8_t cmd)
{
	if(QUEUE_IsFull(queue))
	{
		//return 0;
	}
	else
	{
		queue->buf[(queue->front++) % queue->size] = cmd;
		//return 1;
	}
}

uint8_t QUEUE_delete(EVON_QUEUE* queue)
{
	if(QUEUE_IsEmpty(queue))
	{
		return 0;
	}
	else return queue->buf[(queue->rear++) % queue->size];
}
