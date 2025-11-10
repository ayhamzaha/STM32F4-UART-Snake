#ifndef ADC_H
#define ADC_H

#include "stm32f446xx.h"

/*
Configure ADC1 such that it converts 2 analog inputs
then sends that data over DMA into 2 variables
the ADC should do 2 conversions every trigger
reading both PA0 and PA1
*/

#define VR_X_AXIS 0U // PA0
#define VR_Y_AXIS 1U // PA1

void Init_adc(void);

#endif