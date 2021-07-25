#ifndef RCC_CONFIG_H
#define RCC_CONFIG_H

#include "stm32f4xx.h"
#include "stm32f411xe.h"
#include <stdint.h>

#define PLL_N 100
#define PLL_M 4
#define PLL_P 0 // PLLP = 2

//extern volatile uint32_t Millis;
//extern volatile uint32_t Micros;

void systickConfig(void);
void clockConfig(void);
uint32_t micros(void);
uint32_t millis(void);
void delayMicroseconds(uint32_t time);
void delayMilliseconds(uint32_t time);
void SysTick_Handler(void);

#endif
