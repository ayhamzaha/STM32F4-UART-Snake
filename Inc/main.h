#ifndef MAIN_H
#define MAIN_H

#include "stm32f446xx.h"
#include "timer.h"
#include "queue.h"
#include "uart.h"
#include "adc.h"
#include "dma.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BTN1 3U // PB3
#define BTN2 5U // PB5
#define SCREEN_RFRSH_RATE 200U // in ms
#define BTN_DEBOUNCE_MS 10U

void Init_BTNs(void);
void Refresh_Screen(uint8_t xpos, uint8_t ypos);



#endif