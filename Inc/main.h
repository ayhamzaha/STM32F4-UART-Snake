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

#define SCREEN_RFRSH_RATE 200U // in ms

enum {
	down,
	up,
	right,
	left
} direction;

enum {
	start,
	playing,
	end
} gamestate;

extern char buffer[8][17];
extern uint8_t game_state;
/*
game_state = 0 -> press to start, click to go to game_state = 1
game_state = 1 -> playing, lose to go to game_state = 2
game_state = 2 -> game over, click to go to game_state = 0
*/
extern uint8_t xpos;
extern uint8_t ypos;
extern uint8_t score;
extern uint8_t body_x[64];
extern uint8_t body_y[64];

extern uint8_t dir;
/*
dir = 0 -> moving down (-x)
dir = 1 -> moving up (+x)
dir = 2 -> moving right (+y)
dir = 3 -> moving left (-y)
*/


void Init_BTNs(void);
void Refresh_Screen(uint8_t xpos, uint8_t ypos);



#endif