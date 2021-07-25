#ifndef _IBUS_H_
#define _IBUS_H_

#include "stm32f4xx.h"
#include "stm32f411xe.h"
#include <stdbool.h>

#define IBUS_MAXCHANNELS 14
#define IBUS_BUFFSIZE 32

extern volatile uint16_t rcValue[IBUS_MAXCHANNELS];
extern volatile bool rxFrameDone;

void USART1_IRQHandler(void);

#endif
