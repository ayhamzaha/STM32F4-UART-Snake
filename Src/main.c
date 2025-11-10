#include "main.h"

char buffer[SCREEN_SIZE][(SCREEN_SIZE * 2U) + 1U];
char stage_row[(SCREEN_SIZE*2U) + 1U];
uint32_t seed = 0;
uint8_t game_state = start;
/*
game_state = 0 -> press to start, click to go to game_state = 1
game_state = 1 -> playing, lose to go to game_state = 2
game_state = 2 -> game over, click to go to game_state = 0
*/
uint8_t xpos = 0;
uint8_t ypos = 0;
uint8_t score = 0;
uint8_t body_x[SCREEN_SIZE * SCREEN_SIZE];
uint8_t body_y[SCREEN_SIZE * SCREEN_SIZE];

uint8_t dir = none;
/*
dir = 0 -> moving down (-x)
dir = 1 -> moving up (+x)
dir = 2 -> moving right (+y)
dir = 3 -> moving left (-y)
*/
uint16_t v1 = 0;
uint16_t v2 = 0;
uint8_t flag = 0;

void ADC_IRQHandler(void) { //read 1st conv. then read 2nd conv.
	if(!flag) v1 = (uint16_t)ADC1->DR;
	else v2 = (uint16_t)ADC1->DR;
	flag = ~flag;
}
											

int main(void) {
	Init_adc();
	Init_Timer_us();
	Init_Timer_ms();
	// set stage for custom sizing
	for(unsigned i=0;i < (SCREEN_SIZE*2U);i++){
		if(i%2==0) stage_row[i] = '-';
		else stage_row[i] = ' ';
	}
	stage_row[(SCREEN_SIZE * 2U)] = '\n';
	Init_USART2(921600);
	Q_Init(&TxQ);
	GPIOA->MODER |= (0x1U << (5U * 2U));
	while(1) {
		seed = Generate_seed();
		Read_input();
		if(dir != none){ 
		game_state = playing;
		srand(seed);
		break;
		}
		Refresh_Screen(xpos,ypos);
		Delay_ms(SCREEN_RFRSH_RATE);
	}
	while(game_state == playing) {
		Read_input();
		Refresh_Screen(xpos,ypos);
		Delay_ms(SCREEN_RFRSH_RATE);
	}
	while(1) {
		GPIOA->ODR |= (0x1U << 0x5U);
		Refresh_Screen(xpos,ypos);
		Delay_ms(5000U);
		NVIC_SystemReset();
	} // Game over - reset system
}

uint32_t Generate_seed(void) {
	uint32_t seed = TIM2->CNT ^ (TIM5->CNT << 16U);
	return seed ^ (seed >> 11U) ^ (seed << 7U);
}

void Read_input(void) {
		ADC1->CR2 |= ADC_CR2_SWSTART; // read inputs
		Delay_ms(2); // ensure both inputs are read
//    sprintf(buffer[0],"xpos: %u\nypos: %u\ndir: %u\n",xpos,ypos,dir);
//    USART_Q_Transmit_NonBlocking(&TxQ,buffer[0],strlen(buffer[0]));
// 		Debugging prints ^^^
		if (((v1 < 1500) || (v1 > 3500))) { // check x axis
			if((v1 < 1500) && (dir != down)) { 
				dir = up;
			}
			else if(dir != up){ 
				dir = down;
			}
		}
		else if((v2 < 1500) || (v2 > 3500)) {
			if((v2 < 1500) && (dir != left)){ 
				dir = right;
			}
			else if(dir != right){ 
				dir = left;
			}
		}
		switch(dir){
			case down: 
				xpos = (xpos + 1) % SCREEN_SIZE;
				break;
			
			case up:
				xpos--;
				if(xpos >= SCREEN_SIZE) xpos = SCREEN_SIZE-1U;
				break;
			
			case left:
				ypos--;
				if(ypos >= SCREEN_SIZE) ypos = SCREEN_SIZE-1U;
				break;
			
			case right:
				ypos = (ypos + 1) % SCREEN_SIZE;
				break;
			
			default:
				break; // when dir == none
		}
}

void Refresh_Screen(uint8_t xpos, uint8_t ypos) {
		
		if(game_state == start) USART_Q_Transmit_NonBlocking(&TxQ,"Move to start!\n",16);
		else if(game_state == playing) USART_Q_Transmit_NonBlocking(&TxQ,"SNAKE\n",7);
		else if(game_state == end) USART_Q_Transmit_NonBlocking(&TxQ,"GAME OVER\n",10);
		body_x[0] = xpos;
		body_y[0] = ypos;
		static uint8_t r_xnum = SCREEN_SIZE/2U;
		static uint8_t r_ynum = SCREEN_SIZE/2U;
	
		// Displays player score above stage
		sprintf(buffer[0],"Score: %u\n",score);
		USART_Q_Transmit_NonBlocking(&TxQ,buffer[0],strlen(buffer[0]));
		
		// Reset all stage rows that have been modified
		strcpy(buffer[0],stage_row);
		for(uint8_t i = 0; i < SCREEN_SIZE; i++) {
			if(!strcmp(buffer[i],stage_row)) continue;
			strcpy(buffer[i],stage_row);
		}
		
		// Find new fruit location and update score
		if(body_x[0] == r_xnum && body_y[0] == r_ynum) {
			while(r_xnum == body_x[0]) {
				r_xnum = (rand() % SCREEN_SIZE);
			}
			r_ynum = (rand() % SCREEN_SIZE);
			score++;
		}
		
		// Find new snake head/body location and check for game over (collision)
		for(unsigned i=0; i < score; i++) { // 1 score = 1 snake body part
			if(body_x[score - i] != body_x[(score - i) - 1]) { // moved on x-axis
				buffer[body_x[score - i]][body_y[(score - i) - 1]*2U] = SNAKE_BODY; // place body of the snake
			}
			else if(body_y[score - i] != body_y[(score - i) - 1]){ // moved on y-axis
				buffer[body_x[(score - i) - 1]][body_y[score - i]*2U] = SNAKE_BODY; // place body of the snake
			}
			if((body_x[0] == body_x[i+1]) && (body_y[0] == body_y[i+1])) { // body + head collide = game over
				game_state = end; 
				return;
			}
		}
		
		// Print new fruit and snake head locations on stage
		buffer[r_xnum][r_ynum*2U] = SNAKE_FRUIT; // fruit
		buffer[body_x[0]][body_y[0]*2U] = SNAKE_HEAD; // print head of the snake
		USART_Q_Transmit_NonBlocking(&TxQ, buffer[0], strlen(buffer[0]));
		
		// Print snake body and rest of stage
		USART_Q_Transmit_NonBlocking(&TxQ,"--------------------------------\n",33);
		
		// Update snake body positions
		for(unsigned i=0; i < score; i++) { 
			body_x[score - i] = body_x[(score - i) - 1];
			body_y[score - i] = body_y[(score - i) - 1];
		}
}
