#include "stm32f411xe.h"
#include "stm32f4xx.h"


#include "RCC_Config.h"
#include "timer.h"
#include "usart.h"
#include "SPI.h"
#include "LED_Config.h"
#include "ibus.h"
#include "mpu6000.h"
#include "complementary_filter.h"
#include "MahonyAHRS.h"
#include "pid.h"
#include "mixer.h"
#include "task.h"
#include "global.h"

void sysSetup(void) {
	
	// initialize clock
	clockConfig();
	
	// initialize usart 1 to use ibus
	USART1Init();
	
	// initialize debug port
	#ifdef DEBUG_UART
	USART2Init();
	#endif
	
	// initialize SPI1 to use MPU6000
	SPI1Init();
	
	// initialize LEDs
	ledSetup();
	
	// initialize dshot protocol
	DshotInit();
	
}

void sensorCheck(void) {
	
	// give up if we dont find an IMU
	while (!isPresentMPU6000()) {
		delayMilliseconds(1000);
		LED_ON;
		delayMilliseconds(300);
		LED_OFF;
		delayMilliseconds(300);
		LED_ON;
		delayMilliseconds(300);
		LED_OFF;
	}
	
}

void sensorInit(void) {
	initMPU6000();
}

void sensorCalibrate(void) {
	
	// wait for 5 seconds the board to fully stand still
	for (int i = 0; i < 1250; i += 1) {
		if (i % 5 == 0)
			LED_TOGGLE;
		delayMilliseconds(4);
	}
	
	calibrateMPU6000();
	
}

void throttleCheck(void) {
		
	while (rcValue[2] > 1100) {
		delayMilliseconds(1000);
		LED_ON;
		delayMilliseconds(100);
		LED_OFF;
		delayMilliseconds(100);
		LED_ON;
		delayMilliseconds(100);
		LED_OFF;
		delayMilliseconds(100);
		LED_ON;
		delayMilliseconds(100);
		LED_OFF;
	}
	
}

void armESC(void) {
		
	delayMilliseconds(500);
	DshotIssueCommand(2047, 2047, 2047, 2047);
	delayMilliseconds(500);
	DshotIssueCommand(48, 48, 48, 48);

}

void flightProcedure(void) {
	
	readMPU6000();
	getGyroRates();
	MadgwickAHRSupdateIMU(gyro_x_rate * (M_PI / 180.0f), gyro_y_rate * (M_PI / 180.0f), gyro_z_rate * (M_PI / 180.0f), acc_x, acc_y, acc_z);
	armProcedure();
	pidCalculate();
	outputMotors();
	
}
