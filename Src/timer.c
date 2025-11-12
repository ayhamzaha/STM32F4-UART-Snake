#include "timer.h"


void Delay_ms(uint32_t ms) {
	TIM5->CNT = 0;
	while(TIM5->CNT < ms) __ASM("NOP");
}

void Init_Timer_ms(void) {
	// 1 ms per tick timer
	// Enable peripheral clock (TIM5)
	RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
	TIM5->PSC = 15999U;
	TIM5->ARR = 0xFFFFFFFEU;
	TIM5->EGR |= TIM_EGR_UG;
	TIM5->CR1 |= TIM_CR1_CEN;
}