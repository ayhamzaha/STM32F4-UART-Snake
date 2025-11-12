#ifndef TIMER_H
#define TIMER_H

/*
Initializes timer used for ms delay and seed RNG
*/



#include "stm32f446xx.h"

void Init_Timer_ms(void);
void Delay_ms(uint32_t ms);


#endif