#ifndef _USART_H_
#define _USART_H_

#include "stm32f4xx.h"
#include "stm32f411xe.h"

#include <stdint.h>
#include "string.h"
#include "stdlib.h"
#include "stdarg.h"

void USART1Init(void);
void USART2Init(void);
void printMsg(char *msg, ...);
//void USART1_IRQHandler(void);

#endif
