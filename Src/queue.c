#include "../Inc/queue.h"
#include <stdint.h>
#include "cmsis_gcc.h"

int Q_Enqueue(Q_T *q, uint8_t d){
  uint32_t masking_state;
  // If queue is full, do not overwrite data, but return error code
  if(!Q_Full(q)){
    q->data[q->tail] = d;
    // Protect operations from preemption
    // Save current masking state
    masking_state = __get_PRIMASK();
    // Disable interrupts
    __disable_irq();
    // Update variables
    q->tail = (q->tail+1) % Q_MAX_SIZE;
    q->size++;
    // Restore the interrupt masking state
    __set_PRIMASK(masking_state);
    return 1; // Success 
  } else {
    return 0; // Fail
  }
}

uint8_t Q_Dequeue(Q_T *q){
  uint32_t masking_state;
  uint8_t t = 0;
  // Make sure queue is not empty
  if(!Q_Empty(q)){
    t = q->data[q->head];
    q->data[q->head] = '_'; // Empty unused entries for debugging
    // Protect operations from preemption
    // Save current masking state
    masking_state = __get_PRIMASK();
    // Disable interrupts
    __disable_irq();
    // Update variables
    q->head = (q->head + 1) % Q_MAX_SIZE;
	q->size--;
	// Restore interrupts
	__set_PRIMASK(masking_state);
  }
  return t;
}

int Q_Full(Q_T *q) {
	// 1 = is full
	return q->size == Q_MAX_SIZE;
}

int Q_Empty(Q_T *q) {
	// 1 = is empty
	return q->size == 0;
}

void Q_Init(volatile Q_T *q) {
	unsigned int i;
	// Set default values to 0 for debugging
	for(i = 0; i < Q_MAX_SIZE; ++i) {
		q->data[i] = 0;
	}
	q->head = 0;
	q->tail = 0;
	q->size = 0;
}