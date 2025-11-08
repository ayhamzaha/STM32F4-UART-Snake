#ifndef UART_H
#define UART_H

#include <stdint.h>
#include "stm32f446xx.h"
#include "queue.h"

#define TX_PIN 2U // PA2
#define RX_PIN 3U // PA3

extern Q_T TxQ;

void Init_USART2(uint32_t baud);
void USART_Q_Transmit_NonBlocking(Q_T *q, char * buf, uint8_t size);




#endif