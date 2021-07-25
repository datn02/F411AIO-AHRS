#include "stm32f4xx.h"
#include "stm32f411xe.h"
#include "timer.h"
#include <stdio.h>



static uint16_t pulseTIM3CH1[DSHOT_BUFFER_LENGTH];
static uint16_t pulseTIM3CH2[DSHOT_BUFFER_LENGTH];
static uint16_t pulseTIM4CH1[DSHOT_BUFFER_LENGTH];
static uint16_t pulseTIM4CH2[DSHOT_BUFFER_LENGTH];

/*
void TIM3Init(void) {
	//RCC->APB1ENR |= (1 << 0);
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	
	// Prescaler = 10 -> 9 
	TIM3->PSC = 99;
	
	// Max ARR
	TIM3->ARR = 0xffff;
	
	// Enable timer
	TIM3->CR1 |= TIM_CR1_CEN;
	while(!(TIM3->SR & (1 << 0)));
}
*/


void DshotInit(void) {
	/*-------*/
	/*
	B04 			DMA1_Stream4, Channel 5, TIM3 CCR1
	B05 			DMA1_Stream5, Channel 5, TIM3 CCR2
	B06				DMA1_Stream0, Channel 2, TIM4 CCR1
	B07				DMA1_Stream3, Channel 2, TIM4 CCR2
	*/
	
	// Enable GPIO clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	
	// Enable timer clock
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM4EN;
	
	// Enable DMA clock
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
	
	// GPIO Alternate function mode
	GPIOB->MODER &= ~((1 << 8) | (1 << 10) | (1 << 12) | (1 << 14));
	GPIOB->MODER |= (1 << 9) | (1 << 11) | (1 << 13) | (1 << 15);
	
	// High-speed output
	GPIOB->OSPEEDR |= (3 << 8) | (3 << 10) | (3 << 12) | (3 << 14);
	
	// No pull
	GPIOB->PUPDR &= ~((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14));
	
	// GPIO alternate function 2
	GPIOB->AFR[0] |= (2 << 16) | (2 << 20) | (2 << 24) | (2 << 28);
	
	/* TIM3 base initialization */
	
	// Disable timer 3
	TIM3->CR1 &= ~TIM_CR1_CEN;
	
	// Prescaler
	TIM3->PSC = (uint16_t)(((float) 100000000 / MOTOR_DSHOT600_HZ + 0.01f) - 1);
	
	// ARR
	TIM3->ARR = DSHOT_PERIOD - 1;

	/* TIM4 base initialization */
	
	// Disable timer 4
	TIM4->CR1 &= ~TIM_CR1_CEN;
	
	// Presecaler
	TIM4->PSC = (uint16_t)(((float) 100000000 / MOTOR_DSHOT600_HZ + 0.01f) - 1);
	
	// ARR
	TIM4->ARR = DSHOT_PERIOD - 1;
	
	/* TIM3 OC initialization */
	
	// DMA request on capture compare chan. 1 and 2
	TIM3->DIER |= TIM_DIER_CC1DE | TIM_DIER_CC2DE;
	
	// PWM mode 1, OC preload enabled
	TIM3->CCMR1 |= (6 << 12) | (1 << 11) | (6 << 4) | (1 << 3);
	
	// Initialize CCRx to 0
	TIM3->CCR1 = 0;
	TIM3->CCR2 = 0;
	
	// Enable OC output
	TIM3->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;
	
	/* TIM4 OC Initialization */
	
	// DMA request on compare match chan. 1 and 2
	TIM4->DIER |= TIM_DIER_CC1DE | TIM_DIER_CC2DE;
	
	// PWM moed 1, OC preload enabled
	TIM4->CCMR1 |= (6 << 12) | (1 << 11) | (6 << 4) | (1 << 3);
	
	// Initialize CCRx to 0
	TIM4->CCR1 = 0;
	TIM4->CCR2 = 0;

	// Enable OC output
	TIM4->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;
	
	// Enable timer
	TIM3->CNT = 0;
	TIM4->CNT = 0;
	
	TIM3->EGR |= TIM_EGR_UG;
	TIM4->EGR |= TIM_EGR_UG;
	
	// Enable counter
	TIM3->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN;
	TIM4->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN;
	
	
	/* DMA initialization */
	
	// Disable all 4 streams
	DMA1_Stream4->CR &= ~(1 << 0);
	while(DMA1_Stream4->CR & 0x00000001);
	
	DMA1_Stream5->CR &= ~(1 << 0);
	while(DMA1_Stream5->CR & 0x00000001);
	
	DMA1_Stream0->CR &= ~(1 << 0);
	while(DMA1_Stream0->CR & 0x00000001);
	
	DMA1_Stream3->CR &= ~(1 << 0);
	while(DMA1_Stream3->CR & 0x00000001);
	

	// Set priority 
	NVIC_SetPriority(DMA1_Stream4_IRQn, 3);
	NVIC_EnableIRQ(DMA1_Stream4_IRQn);
	
	NVIC_SetPriority(DMA1_Stream5_IRQn, 3);
	NVIC_EnableIRQ(DMA1_Stream5_IRQn);
	
	NVIC_SetPriority(DMA1_Stream0_IRQn, 3);
	NVIC_EnableIRQ(DMA1_Stream0_IRQn);
	
	NVIC_SetPriority(DMA1_Stream3_IRQn, 3);
	NVIC_EnableIRQ(DMA1_Stream3_IRQn);
	
	
	/* DMA TIM3 CCR1 - Stream 4 Channel 5 */
	
	// Channel 5 select, interrupt enabled
	DMA1_Stream4->CR |= (5 << 25) | (3 << 16) | (1 << 13) | (1 << 11) | (1 << 10) | (1 << 6) | (1 << 4);
	
	// 18 
	DMA1_Stream4->NDTR = DSHOT_BUFFER_LENGTH;
	
	// TIM3 CCR1 address
	DMA1_Stream4->PAR = (uint32_t) &(TIM3->CCR1);
	
	// Address of pulse data
	DMA1_Stream4->M0AR = (uint32_t) pulseTIM3CH1;
	
	
	/* DMA TIM3 CCR2 - Stream 5 Channel 5 */
	
	// Channel 5 select, interrupt enabled
	DMA1_Stream5->CR |= (5 << 25) | (3 << 16) | (1 << 13) | (1 << 11) | (1 << 10) | (1 << 6) | (1 << 4);

	// 18
	DMA1_Stream5->NDTR = DSHOT_BUFFER_LENGTH;
	
	// TIM3 CCR2 address
	DMA1_Stream5->PAR = (uint32_t) &(TIM3->CCR2);
	
	// Address of pulse data
	DMA1_Stream5->M0AR = (uint32_t) pulseTIM3CH2;
	
	
	/* DMA TIM4 CCR1 - Stream 0 Channel 2 */
	
	// Channel 2 select, interrupt enabled
	DMA1_Stream0->CR |= (2 << 25) | (3 << 16) | (1 << 13) | (1 << 11) | (1 << 10) | (1 << 6) | (1 << 4);

	// 18
	DMA1_Stream0->NDTR = DSHOT_BUFFER_LENGTH;
	
	// TIM4 CCR1 address
	DMA1_Stream0->PAR = (uint32_t) &(TIM4->CCR1);
	
	// Adress of pulse data
	DMA1_Stream0->M0AR = (uint32_t) pulseTIM4CH1;


	/* DMA TIM4 CCR2 - Stream 3 Channel 2 */
	
	// Channel 2 select, interrupt enabled
	DMA1_Stream3->CR |= (2 << 25) | (3 << 16) | (1 << 13) | (1 << 11) | (1 << 10) | (1 << 6) | (1 << 4);
	
	// 18
	DMA1_Stream3->NDTR = DSHOT_BUFFER_LENGTH;
	
	// TIM4 CCR2 address
	DMA1_Stream3->PAR = (uint32_t) &(TIM4->CCR2);
	
	// Address of pulse data
	DMA1_Stream3->M0AR = (uint32_t) pulseTIM4CH2;

}

uint16_t prepareDshotPacket(uint16_t value, bool telemtry) {
	
	uint16_t packet = (uint16_t) (value << 1) | (telemtry ? 1 : 0);
	telemtry = false;
	
	// compute checksum
	uint16_t csum = 0;
	uint16_t csum_data = packet;
	
	for (int i = 0; i < 3; i += 1) {
		csum ^= csum_data;
		csum_data >>= 4;
	}
	
	// only need last 4 LSBs for checksum data
	csum &= 0x000f;
	
	// add checksum data into data packet
	packet = (uint16_t) (packet << 4) | csum;
	
	return packet;
	
}


void DshotIssueCommand(uint16_t command1, uint16_t command2, uint16_t command3, uint16_t command4) {
	
	uint16_t packet1 = prepareDshotPacket(command1, false);
	uint16_t packet2 = prepareDshotPacket(command2, false);
	uint16_t packet3 = prepareDshotPacket(command3, false);
	uint16_t packet4 = prepareDshotPacket(command4, false);
	
	
	// wait for all streams disbabled
	while(DMA1_Stream4->CR & 0x00000001);
	while(DMA1_Stream5->CR & 0x00000001);
	while(DMA1_Stream0->CR & 0x00000001);
	while(DMA1_Stream3->CR & 0x00000001);
	
	
	// load DMA CCRx buffer 
	for (int i = 0; i < 16; i += 1) {
		pulseTIM3CH1[i] = (packet1 & 0x8000) ? DSHOT_BIT_1 : DSHOT_BIT_0;
		pulseTIM3CH2[i] = (packet2 & 0x8000) ? DSHOT_BIT_1 : DSHOT_BIT_0;
		pulseTIM4CH1[i] = (packet3 & 0x8000) ? DSHOT_BIT_1 : DSHOT_BIT_0;
		pulseTIM4CH2[i] = (packet4 & 0x8000) ? DSHOT_BIT_1 : DSHOT_BIT_0;
		
		packet1 <<= 1;
		packet2 <<= 1;
		packet3 <<= 1;
		packet4 <<= 1;
	}
	pulseTIM3CH1[16] = 0;
	pulseTIM3CH2[16] = 0;
	pulseTIM4CH1[16] = 0;
	pulseTIM4CH2[16] = 0;
	pulseTIM3CH1[17] = 0;
	pulseTIM3CH2[17] = 0;
	pulseTIM4CH1[17] = 0;
	pulseTIM4CH2[17] = 0;
	
	// Reset the number of data registers to transfer
	DMA1_Stream4->NDTR = DSHOT_BUFFER_LENGTH;
	DMA1_Stream5->NDTR = DSHOT_BUFFER_LENGTH;
	DMA1_Stream0->NDTR = DSHOT_BUFFER_LENGTH;
	DMA1_Stream3->NDTR = DSHOT_BUFFER_LENGTH;
	
	// Enable DMA streams
	DMA1_Stream4->CR |= (1 << 0);
	DMA1_Stream5->CR |= (1 << 0);
	DMA1_Stream0->CR |= (1 << 0);
	DMA1_Stream3->CR |= (1 << 0);
	
	// Re-initialize counters to 0
	TIM3->CNT = 0;
	TIM4->CNT = 0;
	
	// Re-enable DMA request on compare match
	TIM3->DIER |= TIM_DIER_CC1DE | TIM_DIER_CC2DE;
	TIM4->DIER |= TIM_DIER_CC1DE | TIM_DIER_CC2DE;
	
}


uint16_t convertPWMtoDshot(uint16_t val) {
	uint16_t res, in_min, in_max, out_max, out_min;
	
	in_min = 1000;
	in_max = 2000;
	out_min = 48;
	out_max = 2047;
	
	res = (uint16_t) (((float)(val - in_min) * (out_max - out_min) / (in_max - in_min) + 0.01f) + out_min);
	
	if (res > out_max) {
		res = out_max;
	}
	
	else if (res < out_min) {
		res = out_min;
	}
	
	return res;
	
}


// IRQ handlers
void DMA1_Stream4_IRQHandler(void) {
	
	// transfer complete flag
	if (DMA1->HISR & DMA_HISR_TCIF4) {
		
		// disbale stream
		DMA1_Stream4->CR &= ~(1 << 0);
		
		// disable DMA request on timer
		TIM3->DIER &= ~TIM_DIER_CC1DE;
		
		// clear interrupt flag
		DMA1->HIFCR = DMA_HIFCR_CTCIF4;
		
	}
}

void DMA1_Stream5_IRQHandler(void) {
	
	// transfer complete flag
	if (DMA1->HISR & DMA_HISR_TCIF5) {
		
		// disable stream
		DMA1_Stream5->CR &= ~(1 << 0);
		
		// disable DMA request on timer
		TIM3->DIER &= ~TIM_DIER_CC2DE;
		
		// clear interrupt flag
		DMA1->HIFCR = DMA_HIFCR_CTCIF5;
		
	}
}

void DMA1_Stream0_IRQHandler(void) {
	
	// transfer complete flag
	if (DMA1->LISR & DMA_LISR_TCIF0) {
		
		// disable stream
		DMA1_Stream0->CR &= ~(1 << 0);
		
		// disable DMA request on timer
		TIM4->DIER &= ~TIM_DIER_CC1DE;
		
		// clear interrupt flag
		DMA1->LIFCR = DMA_LIFCR_CTCIF0;
		
	}
}

void DMA1_Stream3_IRQHandler(void) {
	
	// transfer complete flag
	if (DMA1->LISR & DMA_LISR_TCIF3) {
		
		// disable stream
		DMA1_Stream3->CR &= ~(1 << 0);
		
		// disable DMA request on timer
		TIM4->DIER &= ~TIM_DIER_CC2DE;
		
		// clear interupt flag
		DMA1->LIFCR = DMA_LIFCR_CTCIF3;
		
	}
}

