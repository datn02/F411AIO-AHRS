#ifndef _TIMER_H_
#define _TIMER_H_

#include "stm32f4xx.h"
#include "stm32f411xe.h"
#include <stdbool.h>

#define DSHOT_PERIOD 20
#define DSHOT_COMMAND_LENGTH 16
#define DSHOT_BUFFER_LENGTH	18
#define DSHOT_BIT_0 7
#define DSHOT_BIT_1 14

#define MHZ_TO_HZ(x) ((x) * 1000000)
#define MOTOR_DSHOT1200_HZ    MHZ_TO_HZ(24)
#define MOTOR_DSHOT600_HZ     MHZ_TO_HZ(12)
#define MOTOR_DSHOT300_HZ     MHZ_TO_HZ(6)
#define MOTOR_DSHOT150_HZ     MHZ_TO_HZ(3)


void DshotInit(void);
uint16_t prepareDshotPacket(uint16_t value, bool telemetry);

//extern uint16_t motorPulseTrain[18];

//void TIM3Init(void);
//void TIM4Init(void);
//void DMA_DeInit(DMA_Stream_TypeDef* DMAy_Streamx);
//void TIMDelayMicro(uint32_t time);
//void TIMDelayMs(uint32_t time);
void DshotIssueCommand(uint16_t command1, uint16_t command2, uint16_t command3, uint16_t command4);
uint16_t convertPWMtoDshot(uint16_t val);
void DMA1_Stream4_IRQHandler(void);
void DMA1_Stream5_IRQHandler(void);
void DMA1_Stream0_IRQHandler(void);
void DMA1_Stream3_IRQHandler(void);

#endif
