#include "main.h"

char buffer[8][17] = {{'-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-','\n'},
										  {'-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-','\n'},
											{'-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-','\n'},
											{'-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-','\n'},
											{'-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-','\n'},
									 	  {'-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-','\n'},
					 					  {'-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-','\n'},
						  				{'-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-','\n'}};

uint8_t game_state = start;
/*
game_state = 0 -> press to start, click to go to game_state = 1
game_state = 1 -> playing, lose to go to game_state = 2
game_state = 2 -> game over, click to go to game_state = 0
*/
uint8_t xpos = 0;
uint8_t ypos = 0;
uint8_t score = 0;
uint8_t body_x[64];
uint8_t body_y[64];

uint8_t dir = 0;
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
	Init_USART2(921600);
	Q_Init(&TxQ);
	GPIOA->MODER |= (0x1U << (5U * 2U));
	while(1) {
		if(game_state == playing) break;
		if((v1 > 0) || (v2 > 0)) game_state = playing;
		Refresh_Screen(xpos,ypos);
		Delay_ms(1000);
	}
	while(game_state == playing) {
		if(game_state == end) {
			NVIC_SystemReset();
		}
		ADC1->CR2 |= ADC_CR2_SWSTART; // read inputs
		Delay_ms(5);
		if (((v1 < 1500) || (v1 > 3500))) { // check x axis
			if((v1 < 1500) && (dir != down)) dir = up;
			else if(dir != up) dir = down;
		}
		else if((v2 < 1500) || (v2 > 3500)) {
			if((v2 < 1500) && (dir != left)) dir = right;
			else if(dir != right) dir = left;
		}
		switch(dir) {
			case down: // down
				xpos = (xpos + 1) % 8;
				break;
			case up: // up
				xpos--;
				if(xpos >= 8) xpos = 7;
				break;
			case right: // right
				ypos = (ypos + 1) % 8;
				break;
			case left: // left
				ypos--;
				if(ypos >= 8) ypos = 7;
				break;
		}
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

void Refresh_Screen(uint8_t xpos, uint8_t ypos) {
	
		if(game_state == start) USART_Q_Transmit_NonBlocking(&TxQ,"Move to start!\n",21);
		else if(game_state == playing) USART_Q_Transmit_NonBlocking(&TxQ,"SNAKE\n",7);
		else if(game_state == end) USART_Q_Transmit_NonBlocking(&TxQ,"GAME OVER\n",10);
		body_x[0] = xpos;
		body_y[0] = ypos;
		srand(TIM2->CNT);
		srand(TIM5->CNT);
		static uint8_t r_xnum = 4;
		static uint8_t r_ynum = 0;
	
	
		sprintf(buffer[0],"Score: %u\n",score);
		USART_Q_Transmit_NonBlocking(&TxQ,buffer[0],strlen(buffer[0]));
		strcpy(buffer[0],"- - - - - - - -\n");
		
		for(uint8_t i = 0; i < 8; i++) {
			if(!strcmp(buffer[i],"- - - - - - - -\n")) continue;
			strcpy(buffer[i],"- - - - - - - -\n");
		}
		
		if(body_x[0] == r_xnum && body_y[0] == r_ynum) {
			while(r_xnum == body_x[0]) {
				r_xnum = (rand() % 8);
			}
			r_ynum = (rand() % 8);
			score++;
		}
		// print snake body
		for(unsigned i=0; i < score; i++) { // 1 score = 1 body part
			if((body_x[0] == body_x[i]) && (body_y[0] == body_y[i]) && (i > 0)) { // body + head collide = game over
				game_state = end; 
				return;
			}
			if(body_x[score - i] != body_x[(score - i) - 1]) { // moved on x-axis
				buffer[body_x[score - i]][body_y[(score - i) - 1]*2U] = 'X'; // print body of the snake
			}
			else if(body_y[score - i] != body_y[(score - i) - 1]){ // moved on y-axis
				buffer[body_x[(score - i) - 1]][body_y[score - i]*2U] = 'X'; // print body of the snake
			}
		}
		buffer[r_xnum][r_ynum*2U] = 'O'; // fruit
		buffer[body_x[0]][body_y[0]*2U] = '@'; // print head of the snake
			
		for(uint8_t i = 0; i < 8; i++){
			USART_Q_Transmit_NonBlocking(&TxQ, buffer[i], strlen(buffer[i]));
		}
		
		USART_Q_Transmit_NonBlocking(&TxQ,"----------------\n",18);
		
		for(unsigned i=0; i < score; i++) { //update body positions
			body_x[score - i] = body_x[(score - i) - 1];
			body_y[score - i] = body_y[(score - i) - 1];
		}
}
