#ifndef MAIN_H
#define MAIN_H

#include "stm32f446xx.h"
#include "timer.h"
#include "queue.h"
#include "uart.h"
#include "adc.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
Handles main game logic (refreshing screen, generating seeds)
Reads inputs from ADC to determine snake direction
Allows for different snake stage sizes, characters, and refresh rate
*/

#define SCREEN_RFRSH_RATE 175U // in ms
#define SNAKE_BODY 'O'
#define SNAKE_HEAD 'Q'
#define SNAKE_FRUIT '@'
#define SCREEN_SIZE (8U)
#define INPUT_UPPER_BOUND (3250U)
#define INPUT_LOWER_BOUND (1750U)
#define BAUDRATE (921600U)

typedef enum {
	down,
	up,
	right,
	left,
	none
} direction;

typedef enum {
	start,
	playing,
	end,
	win
} state;

extern char stage[SCREEN_SIZE][(SCREEN_SIZE * 2U) + 1U];
extern char stage_row[(SCREEN_SIZE * 2U) + 1U];
extern uint8_t body_x[SCREEN_SIZE*SCREEN_SIZE];
extern uint8_t body_y[SCREEN_SIZE*SCREEN_SIZE];

uint32_t Generate_seed(void);
direction Read_input(direction dir);
void Update_snake(direction dir, uint8_t * head, state gamestate);
state Refresh_Screen(uint8_t * head, state gamestate);
void Set_stage(void);
void Print_head_ui(state gamestate, uint8_t score);


#endif