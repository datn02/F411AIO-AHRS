#include "stm32f411xe.h"
#include "stm32f4xx.h"
#include "RCC_Config.h"

static volatile uint32_t Millis;
static volatile uint32_t Micros;

void systickConfig(void) {
	
	SystemCoreClock = 100000000;
	
	if (SysTick_Config(SystemCoreClock / 1000))
		while(1);
	
	NVIC_SetPriorityGrouping(5);  // 2 bits for preemption, 2 bits for sub-priority
	// NVIC_SetPriorityGrouping(3) // Use this for 4 bits of preemption, 0 bits of sub-priorities
	//////////// EXPLANATION /////////////
	/*
	By default, the priority grouping register of STM32F4 is 8-bit wide, but the actual priority
	settings in our code requires the first 4 MSBs of this register
	refer to https://www.keil.com/pack/doc/CMSIS/Core/html/index.html#ref_man_sec
	for the user manual of Cortex-M4 programming
	
	Basically, NVIC_SetPriorityGrouping() of CMISIS core has 8 values (0 to 7) which places the 
	distinguish line for preemption and sub-priority values as mentioned above, but it does so 
	for the whole 8 bits of the priority register. Therefore, we only use the value from 3 to 7 
	for this function.
	
	The NVIC_SetPriority() function however will automatically shifts over 4 bits to fit in the required
	field of priority numbering for us, so no need extra maths here
	*/
	//////////////////////////////////////
	NVIC_SetPriority(SysTick_IRQn, 0);
	
	Millis = 0;
}

uint32_t micros(void) {
	Micros = Millis * 1000 + 1000 - SysTick->VAL/72;
	//Micros = Millis*1000+(SystemCoreClock/1000-SysTick->VAL)/72;
	return Micros;
}

uint32_t millis(void) {
	return Millis;
}

void delayMilliseconds(uint32_t time) {
	uint32_t currTime = millis();
	
	while ((time - (millis() - currTime)) > 0);
}

void delayMicroseconds(uint32_t time) {
	uint32_t currTime = micros();
	
	while ((time - (micros() - currTime)) > 0);
}

void SysTick_Handler(void) {
	Millis++;
}

void clockConfig(void) {

	// Turn on HSE and wait for it to stable
	RCC->CR |= RCC_CR_HSEON;
	while (!(RCC->CR & RCC_CR_HSERDY));
	
	// Set POWER ENABLE CLOCK and VOLTAGE REGULATOR
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	PWR->CR |= PWR_CR_VOS;
	
	// Configure Flash
	FLASH->ACR |= FLASH_ACR_PRFTEN | FLASH_ACR_DCEN | FLASH_ACR_ICEN | FLASH_ACR_LATENCY_3WS;
	
	// Configure prescalers
	// AHB Prescaler = 1
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
	
	// APB1 prescaler = 2 (50Mhz)
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;
	
	// APB2 prescaler = 1 (100Mhz)
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;
	
	// Configure main PLL
	RCC->PLLCFGR = (PLL_M << 0) | (PLL_N << 6) | (PLL_P << 16) | (RCC_PLLCFGR_PLLSRC_HSE);
	
	// Enable PLL and wati for ready
	RCC->CR |= RCC_CR_PLLON;
	while (!(RCC->CR & RCC_CR_PLLRDY));
	
	// Select clock source
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
	
	//SystemCoreClockUpdate();
	//SysTick_Config(SystemCoreClock / 1000);
	systickConfig();
}
