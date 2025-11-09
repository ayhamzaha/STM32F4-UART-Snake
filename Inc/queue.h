#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>

#define Q_MAX_SIZE (256U)

typedef struct {
	uint8_t data[Q_MAX_SIZE];
	unsigned int head;
	unsigned int tail;
	unsigned int size;
} Q_T;


int Q_Enqueue(Q_T *q, uint8_t d);
uint8_t Q_Dequeue(Q_T *q);
int Q_Empty(Q_T *q);
int Q_Full(Q_T *q);
void Q_Init(Q_T *q);

#endif