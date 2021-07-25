#ifndef _SPI_H_
#define _SPI_H_

#include "stm32f411xe.h"
#include "stm32f4xx.h"


void SPI1GPIOInit(void);
void SPI1Init(void);
void SPIEnable(SPI_TypeDef* SPIx);
void SPIDisable(SPI_TypeDef* SPIx);

//uint8_t SPI1Transfer(uint8_t data);
uint8_t SPITransfer(SPI_TypeDef* SPIx, uint8_t data);
void SetSPIClockTo(SPI_TypeDef* SPIx, uint16_t prescaler);

#endif
