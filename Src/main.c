#include "main.h"

char stage[SCREEN_SIZE][(SCREEN_SIZE * 2U) + 1U];
char stage_row[(SCREEN_SIZE*2U) + 1U];
uint32_t seed = 0xABCDEF92;
uint8_t body_x[SCREEN_SIZE * SCREEN_SIZE];
uint8_t body_y[SCREEN_SIZE * SCREEN_SIZE];

uint16_t v1 = 0;
uint16_t v2 = 0;
void ADC_IRQHandler(void) {
	static uint8_t flag = 0;
	if(!flag) v1 = (uint16_t)ADC1->DR;
	else v2 = (uint16_t)ADC1->DR;
	
	// switch ADC pin in group conversion
	flag = ~flag; 
}
											

int main(void) {
	Init_ADC();
	Init_Timer_ms();
	Set_stage();
	Init_USART2(BAUDRATE);
	Q_Init(&TxQ);
	direction dir = none;
	uint8_t snake_head[2] = {0,0}; // {x,y} positions of snake head
	state game_state = start;
	while(1) {
		seed = Generate_seed();
		dir = Read_input(dir);
		if(dir != none){ 
		game_state = playing;
		srand(seed);
		break;
		}
		game_state = Refresh_Screen(snake_head, game_state);
		Delay_ms(SCREEN_RFRSH_RATE);
	}
	while(game_state == playing) {
		dir = Read_input(dir);
		Update_snake(dir,snake_head, game_state);
		game_state = Refresh_Screen(snake_head, game_state);
		Delay_ms(SCREEN_RFRSH_RATE);
	}
	while(1) {
		game_state = Refresh_Screen(snake_head, game_state);
		Delay_ms(5000U);
		NVIC_SystemReset();
	} // Game over / Won -> reset system
}

// Generates pseudo-random seed for fruit locations
uint32_t Generate_seed(void) {
	uint32_t seed = (TIM5->CNT << 16U) ^ (v1 & (v2 << 4U));
	return seed ^ (seed >> 11U) ^ (seed << 7U);
}

void Set_stage(void){
	// set stage for custom sizing
	for(unsigned i=0;i<(SCREEN_SIZE*2U);i++){
		if(i%2==0) stage_row[i] = '-';
		else stage_row[i] = ' ';
	}
	stage_row[(SCREEN_SIZE * 2U)] = '\n';
	
	// Reset all stage rows that have been modified
	for(uint8_t i = 0; i < SCREEN_SIZE; i++) {
		// Do not reset stages that have not been altered
		if(!strcmp(stage[i],stage_row)) continue;
		strcpy(stage[i],stage_row);
	}
}

direction Read_input(direction dir) {
		// Trigger ADC conversion group (PA0 and PA1)
		ADC1->CR2 |= ADC_CR2_SWSTART;
		Delay_ms(5);
		
		// If snake is moving up/down, then it can only move left/right
		// If snake is moving left/right, then it can only move up/down
		// Before game starts, direction = none, so check for that case
		if((dir == down) || (dir == up)){ // snake is moving U/D atm
			if(v2 > INPUT_UPPER_BOUND) dir = left;
			else if(v2 < INPUT_LOWER_BOUND) dir = right;
		}
		else if((dir == left) || (dir == right)){ // snake is moving L/R atm
			if(v1 > INPUT_UPPER_BOUND) dir = down;
			else if(v1 < INPUT_LOWER_BOUND) dir = up;

		} else { // snake is not moving (start of game)
			if(v1 > INPUT_UPPER_BOUND || v1 < INPUT_LOWER_BOUND){
				if(v1 > INPUT_UPPER_BOUND) dir = down;
				else dir = up;
				//v1 > INPUT_UPPER_BOUND ? dir = down : up;
			}
			else if(v2 > INPUT_UPPER_BOUND || v2 < INPUT_LOWER_BOUND){
				if(v2 > INPUT_UPPER_BOUND) dir = left;
				else dir = right;
				//v2 > INPUT_UPPER_BOUND ? dir = left : right;
			}
		}
		return dir;
}

void Update_snake(direction dir, uint8_t * head, state gamestate) {
	
		// Update snake position based on direction
		// Snake moves 1 space constantly in set direction
		if((gamestate != end) && (gamestate != win)){ // if game over do not update snake
			switch(dir){
				case down: 
					head[0] = (head[0] + 1) % SCREEN_SIZE;
					break;
				
				case up:
					head[0]--;
					if(head[0] >= SCREEN_SIZE) head[0] = SCREEN_SIZE-1U;
					break;
				
				case left:
					head[1]--;
					if(head[1] >= SCREEN_SIZE) head[1] = SCREEN_SIZE-1U;
					break;
				
				case right:
					head[1] = (head[1] + 1) % SCREEN_SIZE;
					break;
				
				default:
					break; // when dir == none at start of game
		}
	}
	
}

void Print_head_ui(state gamestate, uint8_t score){
	
		// Print header based on current game state
		if(gamestate == start) USART_Q_Transmit_NonBlocking(&TxQ,"Move to start!\n",15);
		else if(gamestate == playing) USART_Q_Transmit_NonBlocking(&TxQ,"SNAKE\n",7);
		else if(gamestate == end) USART_Q_Transmit_NonBlocking(&TxQ,"GAME OVER\n",10);
		else if(gamestate == win) USART_Q_Transmit_NonBlocking(&TxQ,"YOU WIN!\n",9);
	
		// Display player score above stage
		sprintf(stage[0],"Score: %u\n",score);
		USART_Q_Transmit_NonBlocking(&TxQ,stage[0],strlen(stage[0]));
}

/* 
Responsible for: 
1. Calculating and printing snake/fruit positions
2. Detecting collision (game over condition, fruit collected)
3. Incrementing score on fruit pickup
4. Printing any other UI
5. Checking for win condition (all fruit collected)
*/ 
state Refresh_Screen(uint8_t * head, state gamestate) {
		
	 // Tracks player score
		static uint8_t score = 0;
		Print_head_ui(gamestate, score);
	
		if(score == (SCREEN_SIZE * SCREEN_SIZE)){
			gamestate = win;
		}
	
		// Update snake head position
		body_x[0] = head[0];
		body_y[0] = head[1];
	
		// Static variables for initial fruit location
		static uint8_t r_xnum = SCREEN_SIZE/2U;
		static uint8_t r_ynum = SCREEN_SIZE/2U;

		Set_stage();
		
		// Find new snake head/body location and check for game over (collision)
		for(unsigned i=0; i < score; i++) {
			
			// body + head collide = game over
			if((body_x[0] == body_x[i+1]) && (body_y[0] == body_y[i+1])) 
				gamestate = end;
			
			// update snake body for x-axis movement
			if(body_x[score - i] != body_x[(score - i) - 1]) 
				stage[body_x[score - i]][body_y[(score - i) - 1]*2U] = SNAKE_BODY;
			
			// update snake body for y-axis movement
			else if(body_y[score - i] != body_y[(score - i) - 1])
				stage[body_x[(score - i) - 1]][body_y[score - i]*2U] = SNAKE_BODY;
		}
		
		// Check if snake head collided with fruit
		if(body_x[0] == r_xnum && body_y[0] == r_ynum) {
			// Update fruit location and increment score
			while(r_xnum == body_x[0]) {
				r_xnum = (rand() % SCREEN_SIZE);
			}
			r_ynum = (rand() % SCREEN_SIZE);
			score++;
		}
		
		// Print new fruit and snake head locations on stage
		stage[r_xnum][r_ynum*2U] = SNAKE_FRUIT;
		// If game is over change snake head to highlight collision 
		if(gamestate == end) stage[body_x[0]][body_y[0]*2U] = '!';
		else stage[body_x[0]][body_y[0]*2U] = SNAKE_HEAD;
		
		// Transmit updated game stage and UI
		USART_Q_Transmit_NonBlocking(&TxQ, stage[0], strlen(stage[0]));
		// Transmit divider
		USART_Q_Transmit_NonBlocking(&TxQ,"--------------------------------\n",33);
		
		
		// Update snake body positions
		if(gamestate != end){
			for(unsigned i=0; i < score; i++) { 
				body_x[score - i] = body_x[(score - i) - 1];
				body_y[score - i] = body_y[(score - i) - 1];
			}
	}
		return gamestate;
}
