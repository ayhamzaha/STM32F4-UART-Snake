#include "adc.h"


void Init_ADC(void) {
	// Enable peripheral clock for GPIOA and ADC1
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	
	// Set PA0 and PA1 to analog (11)
	GPIOA->MODER &= ~((0x3U << (VR_X_AXIS*2U)) | (0x3U << (VR_Y_AXIS*2U)));
	GPIOA->MODER |= (0x3U << (VR_X_AXIS*2U)) | (0x3U << (VR_Y_AXIS*2U));
	
	ADC1->SQR1 |= (0x1U << 20U);
	ADC1->SQR3 |= (0U << 0U) | (0x1U << 5U);
	
	ADC1->SMPR2 |= (1U << 0U) | (1U << 3U); // 15 cycles
	
	ADC1->CR1 |= ADC_CR1_SCAN | ADC_CR1_EOCIE; // enables scan mode and EOC int.
	ADC1->CR2 |= ADC_CR2_EOCS;								 // trigger int. after each conv.
	
	// Configure and enable NVIC for ADC
	NVIC_SetPriority(ADC_IRQn, 3);
	NVIC_ClearPendingIRQ(ADC_IRQn);
	NVIC_EnableIRQ(ADC_IRQn);
	
	ADC1->CR2 |= ADC_CR2_ADON;
	ADC1->CR2 |= ADC_CR2_SWSTART;
}