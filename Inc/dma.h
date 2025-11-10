#ifndef DMA_H
#define DMA_H

/*
Configure DMA to work with ADC
which frees up CPU time, needs to
run twice to read both ADC readings
Source: (ADC1->DR)
Destination: (variable in memory)
*/




void Init_DMA(void);


#endif