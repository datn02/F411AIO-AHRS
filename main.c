#include "stm32f411xe.h"
#include "stm32f4xx.h"
#include "global.h"
#include "RCC_Config.h"
#include "LED_Config.h"
#include "timer.h"
#include "usart.h"
#include "ibus.h"
#include "SPI.h"
#include "mpu6000.h"
#include "complementary_filter.h"
#include "MahonyAHRS.h"
#include "pid.h"
#include "task.h"



int main(void) {
	
	sysSetup();
	sensorCheck();
	sensorInit();
	sensorCalibrate();
	//throttleCheck();
	armESC();
	
	uint32_t loop = micros();
	
	while (1) {		
	
		flightProcedure();
	
		while(micros() - loop < LOOPTIME);
		loop = micros();
	
	}
	
	//return 0;
}
