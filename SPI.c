#include "stm32f411xe.h"
#include "stm32f4xx.h"
#include "SPI.h"


void SPI1GPIOInit(void) {
	// Enable GPIOA
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	
	// Alternate function for PA5 (SCK), PA6 (MISO), PA7 (MOSI), and output for PA4 (cs pin)
	GPIOA->MODER |= (1 << 8) | (1 << 11) | (1 << 13) | (1 << 15);
	GPIOA->MODER &= ~((1 << 9) | (1 << 10) | (1 << 12) | (1 << 14));
	
	// Output speed = high
	GPIOA->OSPEEDR |= (3 << 8) | (3 << 10) | (3 << 12) | (3 << 14);
	
	// No pull-up or down
	GPIOA->PUPDR &= ~((1 << 8) | (1 << 9));
	
	// Select alternate function 5 for PA5,6,7
	GPIOA->AFR[0] |= (5 << 20) | (5 << 24) | (5 << 28); 
}


void SPI1Init(void) {
	
	volatile uint8_t dummyread;
	
	// Enable SPI1 clock
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	
	// Enable GPIOA for SPI1
	SPI1GPIOInit();
	
	// Configure Control Register 1 (CR1)
	SPI1->CR1 |= SPI_CR1_CPHA | SPI_CR1_CPOL;	// CPOL = 1; CPHA = 1
	
	// Master mode
	SPI1->CR1 |= SPI_CR1_MSTR;
	
	// SPI clock prescaler = 128 -> 781250Hz clock
	SPI1->CR1 |= (6 << 3); 
	
	// MSB First
	SPI1->CR1 &= ~(1 << 7);
	
	// Software Slave select (CS pin)
	SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI; //
	
	// 8-bit data frame, full-duplex mode
	SPI1->CR1 &= ~((1 << 10) | (1 << 11));
	
	// Not touching the CR2 register
	SPI1->CR2 = 0;
	
	// Enable SPI1
	SPIEnable(SPI1);
	
	// Clear the DR and the TXE flags
	while (!(SPI1->SR & SPI_SR_TXE));
	dummyread = (uint8_t) SPI1->DR;
}

void SPIEnable(SPI_TypeDef* SPIx) {
	SPIx->CR1 |= SPI_CR1_SPE;
}

void SPIDisable(SPI_TypeDef* SPIx) {
	SPIx->CR1 &= ~SPI_CR1_SPE;
}

uint8_t SPITransfer(SPI_TypeDef* SPIx, uint8_t data) {
	uint16_t SPITimeout;
	
	// Set timeout for transmit routine
	SPITimeout = 0x1000;
	
	// Wait for empty and return 0 if timed out
	while (!(SPIx->SR & SPI_SR_TXE)) {
		if ((SPITimeout--) == 0) {
			return 0;
		}
	}
	
	// Load data into SPI data register to transmit
	SPIx->DR = data;
	
	// Reset timeout variable for receive routine
	SPITimeout = 0x1000;
	
	// Wait for the SPI data register to be filled
	while (!(SPIx->SR & SPI_SR_RXNE)) {
		if ((SPITimeout--) == 0) {
			return 0;
		}
	}
	
	// Return the received data
	return (uint8_t)SPIx->DR;
}

void SetSPIClockTo(SPI_TypeDef* SPIx, uint16_t prescaler) {
	
	#define BR_CLEAR_MASK 0xFFC7
	
	uint16_t tempRegister;
	
	SPIDisable(SPIx);
	
	tempRegister = SPIx->CR1;
	
	switch(prescaler) {
		
		case 2:
			tempRegister &= BR_CLEAR_MASK;
			tempRegister |= (0 << 3);
			break;
		
		case 4:
			tempRegister &= BR_CLEAR_MASK;
			tempRegister |= (1 << 3);
			break;
	
		case 8:
			tempRegister &= BR_CLEAR_MASK;
			tempRegister |= (2 << 3);
			break;
		
		case 16:
			tempRegister &= BR_CLEAR_MASK;
			tempRegister |= (3 << 3);
			break;
		
		case 32:
			tempRegister &= BR_CLEAR_MASK;
			tempRegister |= (4 << 3);
			break;
		
		case 64:
			tempRegister &= BR_CLEAR_MASK;
			tempRegister |= (5 << 3);
			break;
		
		case 128:
			tempRegister &= BR_CLEAR_MASK;
			tempRegister |= (6 << 3);
			break;
		
		case 256:
			tempRegister &= BR_CLEAR_MASK;
			tempRegister |= (7 << 3);
			break;
	
	}
	
	SPIx->CR1 = tempRegister;
	
	SPIEnable(SPIx);
	
}


