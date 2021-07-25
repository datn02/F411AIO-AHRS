#include "stm32f411xe.h"
#include "stm32f4xx.h"
#include "LED_Config.h"
#include "global.h"

void ledSetup(void) {

	// Enable GPIOC clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	
	// Set pin as output
	GPIOC->MODER |= (1 << 26) | (1 << 28);
	
	// Set output mode
	GPIOC->OTYPER &= ~((1 << 13) | (1 << 14));
	
	// Output speed
	GPIOC->OSPEEDR |= (1 << 27) | (1 << 29);
	
	// Pull-up pull-down
	GPIOC->PUPDR &= ~((1 << 29) | (1 << 28) | (1 << 27) | (1 << 26));

}
