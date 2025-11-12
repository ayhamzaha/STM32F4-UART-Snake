#ifndef ADC_H
#define ADC_H

#include "stm32f446xx.h"

/*
Configures and enables ADC1 to handle joystick inputs
Group converion of PA0 and PA1, scan mode
Interrupt on each conversion completion
15 cycles per conversion
*/

#define VR_X_AXIS 0U // PA0
#define VR_Y_AXIS 1U // PA1

void Init_ADC(void);

#endif