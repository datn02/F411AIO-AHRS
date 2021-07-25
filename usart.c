#include "stm32f4xx.h"
#include "stm32f411xe.h"
#include "global.h"

#include <stdint.h>
#include "string.h"
#include "stdlib.h"
#include "stdarg.h"
#include <stdio.h>

#include "usart.h"


void USART1Init(void) {
	
	// Enable USART1 clock and GPIOA clock
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	
	// Configure GPIO alternate function output
	GPIOA->MODER |= (1 << 19) | (1 << 21);
	GPIOA->MODER &= ~((1 << 18) | (1 << 20));
	
	// Configure high speed GPIO speed
	GPIOA->OSPEEDR |= (3 << 18) | (3 << 20);
	
	// Configure alternate function mux
	GPIOA->AFR[1] |= (7 << 4) | (7 << 8);
	
	// Enable USART1
	USART1->CR1 &= 0;
	USART1->CR1 |= (1 << 13);
	
	// Program the M-bit to define word length
	USART1->CR1 &= ~(1 << 12);
	
	// Select the baud rate
	USART1->BRR = (4 << 0) | (54 << 4); // 115200 baud rate
	
	// Enable transmit/receive
	//USART1->CR1 |= (1 << 2) | (1 << 3);
	USART1->CR1 |= USART_CR1_RXNEIE | USART_CR1_TE | USART_CR1_RE;
	
	// Set priority for USART interupt
	NVIC_SetPriority(USART1_IRQn, 2);
	
	// Enable NVIC for USART1
	NVIC_EnableIRQ(USART1_IRQn);
	
}


void USART2Init(void) {
	// Enable GPIOA clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	
	// enable usart 2 clock
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	
	// Alternate function A02 and A03
	GPIOA->MODER &= ~((1 << 4) | (1 << 6));
	GPIOA->MODER |= (1 << 5) | (1 << 7);
	
	// High speed
	GPIOA->OSPEEDR |= (3 << 4) | (3 << 6);
	
	// Alternate function mux
	GPIOA->AFR[0] |= (7 << 8) | (7 << 12);
	
	// Enable USART2
	USART2->CR1 &= 0;
	USART2->CR1 |= (1 << 13);
	
	// 8-bit wordlength
	USART2->CR1 &= ~(1 << 12);
	
	// Select the baud rate
	USART2->BRR |= (2 << 0) | (27 << 4); // 115200 baud
	
	// Enable transmit
	USART2->CR1 |= USART_CR1_TE;
}

void printMsg(char *msg, ...) {

	char buff[80];
	
	#ifdef DEBUG_UART
		
	va_list args;
	va_start(args, msg);
	vsprintf(buff, msg, args);
	
	for (int i = 0; i < strlen(buff); i += 1) {
		USART2->DR = buff[i];
		while (!(USART2->SR & USART_SR_TXE));
	}
	
	#endif
	
}
