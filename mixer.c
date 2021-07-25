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
#include "pid.h"
#include "mixer.h"
#include "task.h"
#include "global.h"


static uint16_t esc_1, esc_2, esc_3, esc_4;
static uint16_t dshot_1, dshot_2, dshot_3, dshot_4;

void outputMotors(void) {
	
	uint16_t throttle = rcValue[2];
	
	if (start == 2) {
		if (throttle > 1800) throttle = 1800;
		esc_1 = (uint16_t) throttle - pid_output_pitch + pid_output_roll - pid_output_yaw;        //Calculate the pulse for esc 1 (front-right - CCW).
    esc_2 = (uint16_t) throttle + pid_output_pitch + pid_output_roll + pid_output_yaw;        //Calculate the pulse for esc 2 (rear-right - CW).
    esc_3 = (uint16_t) throttle + pid_output_pitch - pid_output_roll - pid_output_yaw;        //Calculate the pulse for esc 3 (rear-left - CCW).
    esc_4 = (uint16_t) throttle - pid_output_pitch - pid_output_roll + pid_output_yaw;        //Calculate the pulse for esc 4 (front-left - CW).

    if (esc_1 < 1100) esc_1 = 1050;                                                //Keep the motors running.
    if (esc_2 < 1100) esc_2 = 1050;                                                //Keep the motors running.
    if (esc_3 < 1100) esc_3 = 1050;                                                //Keep the motors running.
    if (esc_4 < 1100) esc_4 = 1050;                                                //Keep the motors running.

    if (esc_1 > 2000)esc_1 = 2000;                                                 //Limit the esc-1 pulse to 2000us.
    if (esc_2 > 2000)esc_2 = 2000;                                                 //Limit the esc-2 pulse to 2000us.
    if (esc_3 > 2000)esc_3 = 2000;                                                 //Limit the esc-3 pulse to 2000us.
    if (esc_4 > 2000)esc_4 = 2000;                                                 //Limit the esc-4 pulse to 2000us.
	}
	
	else {
		esc_1 = 1000;                                                                  //If start is not 2 keep a 1000us pulse for ess-1.
    esc_2 = 1000;                                                                  //If start is not 2 keep a 1000us pulse for ess-2.
    esc_3 = 1000;                                                                  //If start is not 2 keep a 1000us pulse for ess-3.
    esc_4 = 1000;                                                                  //If start is not 2 keep a 1000us pulse for ess-4.
	}
	
	dshot_1 = convertPWMtoDshot(esc_1);
	dshot_2 = convertPWMtoDshot(esc_2);
	dshot_3 = convertPWMtoDshot(esc_3);
	dshot_4 = convertPWMtoDshot(esc_4);
	
	DshotIssueCommand(dshot_2, dshot_1, dshot_3, dshot_4);
	
}
