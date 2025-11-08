#include "../Inc/main.h"

char buffer[8][17] = {{'-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-','\n'},
										  {'-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-','\n'},
											{'-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-','\n'},
											{'-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-','\n'},
											{'-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-','\n'},
									 	  {'-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-','\n'},
					 					  {'-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-','\n'},
						  				{'-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-',' ','-','\n'}};

uint8_t game_state = 0;
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

void EXTI3_IRQHandler(void) {
		// BTN1 clicked
	if(!game_state){ 
		game_state = 1; 
		EXTI->PR = (0x1U << BTN1);
		return;
	}
	static uint32_t last_time = 0;
	uint32_t now = TIM5->CNT;
	if((uint32_t)(now - last_time) > BTN_DEBOUNCE_MS){ // 20ms debounce
		last_time = now;
		if((dir == 0) || (dir == 1)) { // moving up/down
			// Since this is btn1 we should move left
			dir = 3;
		}
		else if((dir == 2) || (dir == 3)) { // moving left/right
			// Since this is btn1 we should move up
			dir = 1;
		}
	}
	EXTI->PR = (0x1U << BTN1);
}
											
void EXTI9_5_IRQHandler(void) {
	// BTN2 clicked
	if(!game_state){ 
		game_state = 1; 
		EXTI->PR = (0x1U << BTN2);
		return;
	}
	static uint32_t last_time1 = 0;
	uint32_t now = TIM2->CNT;
	if((uint32_t)(now - last_time1) > (BTN_DEBOUNCE_MS * 1000U)){ // 20ms debounce
		last_time1 = now;
		if((dir == 0) || (dir == 1)) { // moving up/down
			// Since this is btn2 we should move right
			dir = 2;
		}
		else if((dir == 2) || (dir == 3)) { // moving left/right
			// Since this is btn2 we should move down
			dir = 0;
		}
	}
	EXTI->PR = (0x1U << BTN2);
}

int main(void) {
	Init_BTNs();
	Init_Timer_us();
	Init_Timer_ms();
	Init_USART2(921600);
	Q_Init(&TxQ);
	while(1) {
		if(game_state) break;
		Refresh_Screen(xpos,ypos);
		Delay_ms(1000);
	}
	while(game_state) {
		if(game_state == 2) {
			NVIC_SystemReset();
		}
		switch(dir) {
			case 0: // down
				xpos = (xpos + 1) % 8;
				break;
			case 1: // up
				xpos--;
				if(xpos >= 8) xpos = 7;
				break;
			case 2: // right
				ypos = (ypos + 1) % 8;
				break;
			case 3: // left
				ypos--;
				if(ypos >= 8) ypos = 7;
				break;
		}
		Refresh_Screen(xpos,ypos);
		Delay_ms(SCREEN_RFRSH_RATE);
	}
	while(1) {
	
	} //debug incase start_game is 0 somehow
}

void Init_BTNs(void) {
	// Enable peripheral clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	
	// Set to input (00)
	GPIOB->MODER &= ~((0x3U << (BTN1 * 2U)) | (0x3U << (BTN2 * 2U)));
	
	// Enable pull-up resistors (01)
	GPIOB->PUPDR &= ~((0x3U << (BTN1 * 2U)) | (0x3U << (BTN2 * 2U)));
	GPIOB->PUPDR |= (0x1U << (BTN1 * 2U)) | (0x1U << (BTN2 * 2U));
	
	// Enable interrupt for PB3
	SYSCFG->EXTICR[0] |= (0x1U << (12U));
	
	EXTI->IMR |= (0x1U << BTN1);
	EXTI->FTSR |= (0x1U << BTN1);
	
	NVIC_SetPriority(EXTI3_IRQn, 1);
	NVIC_ClearPendingIRQ(EXTI3_IRQn);
	NVIC_EnableIRQ(EXTI3_IRQn);
	
		// Enable interrupt for PB5
	SYSCFG->EXTICR[1] |= (0x1U << (4U));
	
	EXTI->IMR |= (0x1U << BTN2);
	EXTI->FTSR |= (0x1U << BTN2);

	NVIC_SetPriority(EXTI9_5_IRQn, 1);
	NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
	NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void Refresh_Screen(uint8_t xpos, uint8_t ypos) {
	
		if(!game_state) USART_Q_Transmit_NonBlocking(&TxQ,"Press btn to start!\n",21);
		else USART_Q_Transmit_NonBlocking(&TxQ,"SNAKE\n",7);
		
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
				game_state = 2; 
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
