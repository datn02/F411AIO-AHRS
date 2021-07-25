#include "stm32f411xe.h"
#include "stm32f4xx.h"
#include "ibus.h"
#include "usart.h"


volatile uint16_t rcValue[IBUS_MAXCHANNELS];
volatile bool rxFrameDone;
static uint8_t ibusIndex=0;
static uint8_t ibus[IBUS_BUFFSIZE]={0}; 

void USART1_IRQHandler(void) {
		
	uint8_t i;
	uint16_t chksum, rxsum;
	
	rxFrameDone = false;
	
	if (USART1->SR & USART_SR_RXNE) {
		uint8_t val = (uint8_t) USART1->DR;
		
		if(ibusIndex == 0 && val != 0x20) { return; }
    if(ibusIndex == 1 && val != 0x40) { ibusIndex = 0; return; }
    if(ibusIndex < IBUS_BUFFSIZE) ibus[ibusIndex] = val;
    ibusIndex++;
    
    if(ibusIndex == IBUS_BUFFSIZE) 
    {
      ibusIndex = 0;
      chksum = 0xFFFF;
      for (i=0;i<30;i++) chksum -= ibus[i];
      rxsum = (uint16_t) ibus[30] + (ibus[31]<<8);
      if (chksum == rxsum)
      {
        rcValue[0] = (uint16_t) (ibus[ 3]<<8) + ibus[ 2]; 
        rcValue[1] = (uint16_t) (ibus[ 5]<<8) + ibus[ 4];
        rcValue[2] = (uint16_t) (ibus[ 7]<<8) + ibus[ 6];
        rcValue[3] = (uint16_t) (ibus[ 9]<<8) + ibus[ 8];
        rcValue[4] = (uint16_t) (ibus[11]<<8) + ibus[10];
        rcValue[5] = (uint16_t) (ibus[13]<<8) + ibus[12];
        rcValue[6] = (uint16_t) (ibus[15]<<8) + ibus[14];
        rcValue[7] = (uint16_t) (ibus[17]<<8) + ibus[16];      
        rxFrameDone = true;
      }
    }
	}
}
