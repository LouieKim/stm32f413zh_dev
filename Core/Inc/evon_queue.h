/*
 * evon_queue.h
 *
 *  Created on: 2020. 12. 11.
 *      Author: hyeok
 */

#ifndef INC_EVON_QUEUE_H_
#define INC_EVON_QUEUE_H_

typedef struct _EVON_QUEUE
{
	uint8_t front;
	uint8_t rear;
	uint8_t* const buf;
	uint8_t size;
}EVON_QUEUE;

#define newQUEUE(buf){ \
	0, 0, buf, sizeof(buf)/sizeof(uint8_t) \
}

uint8_t QUEUE_IsEmpty(EVON_QUEUE* queue);
uint8_t QUEUE_IsFull(EVON_QUEUE* queue);
void QUEUE_insert(EVON_QUEUE* queue, uint8_t cmd);
uint8_t QUEUE_delete(EVON_QUEUE* queue);


#endif /* INC_EVON_QUEUE_H_ */
