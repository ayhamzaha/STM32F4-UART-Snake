#include "../Inc/uart.h"

Q_T TxQ;

void USART2_IRQHandler(void){
	// Make sure transmit buffer is empty
	if(USART2->SR & USART_SR_TXE) {
		// Writing to data register clears T-E
		if(!Q_Empty(&TxQ)) {
			USART2->DR = Q_Dequeue(&TxQ);
		} else {
			// Disable transmitter interrupt
			USART2->CR1 &= ~(0x1U << USART_CR1_TXEIE_Pos);
		}
	}
}

void Init_USART2(uint32_t baud) {
	// Enable peripheral clocks
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	
	// Set mode for TX and RX pins to AF (10)
	GPIOA->MODER &= ~((0x3U << (TX_PIN*2)) | (0x3U << (RX_PIN*2)));
	GPIOA->MODER |= (0x2U << (TX_PIN*2)) | (0x2U << (RX_PIN*2));
	
	// Set AF register to be USART2 RX and TX (AF7)
	GPIOA->AFR[0] |= (0x7U << (TX_PIN*4)) | (0x7U << (RX_PIN*4));
	
	// Set baud rate
	USART2->BRR = SystemCoreClock/baud;
	
	// No parity
	USART2->CR1 &= ~(0x1U << USART_CR1_PCE_Pos);
	
	// 8 data bits
	USART2->CR1 &= ~(0x1U << USART_CR1_M_Pos);
	
	// 1 stop bit
	USART2->CR2 &= ~(0x3U << USART_CR2_STOP_Pos);
	
	// Enable interrupt generation on transmit/receive
	USART2->CR1 |= (0x1U << 5U) | (0x1U << 7U);
	
	// Enable USART2
	USART2->CR1 |= (0x1U << USART_CR1_RE_Pos) | (0x1U << USART_CR1_TE_Pos);
	USART2->CR1 |= (0x1U << USART_CR1_UE_Pos);
	
	// Configure and enable NVIC for USART2
	NVIC_SetPriority(USART2_IRQn, 2);
	NVIC_ClearPendingIRQ(USART2_IRQn);
	NVIC_EnableIRQ(USART2_IRQn);
}

void USART_Q_Transmit_NonBlocking(Q_T *q, char * buf, uint8_t size) {
	// Loop through all chars to be sent
	while(size > 0) {
		// Busy-wait until queue is not full
		while(Q_Full(q)) __ASM("NOP");
		// Add char to transmit queue
		Q_Enqueue(q, *buf);
		buf++;
		size--;
	}
	// Ensure we will get a transmit buffer empty interrupt
	USART2->CR1 |= (0x1U << USART_CR1_TXEIE_Pos);
}