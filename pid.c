#include "stm32f411xe.h"
#include "stm32f4xx.h"

#include <stdbool.h>
#include <stdint.h>

#include "timer.h"
#include "RCC_Config.h"
#include "usart.h"
#include "SPI.h"

#include "global.h"

#include "LED_Config.h"
#include "ibus.h"
#include "mpu6000.h"
#include "complementary_filter.h"
#include "MahonyAHRS.h"
#include "pid.h"


static const float pid_p_gain_roll = 0.5f;
static const float pid_i_gain_roll = 0.005f;
static const float pid_d_gain_roll = 10.0f;
static const int pid_max_roll = 300;

static const float pid_p_gain_pitch = 0.5f;
static const float pid_i_gain_pitch = 0.005f;
static const float pid_d_gain_pitch = 10.0f;
static const int pid_max_pitch = 300;

static const float pid_p_gain_yaw = 3.0f;
static const float pid_i_gain_yaw = 0.01f;
static const float pid_d_gain_yaw = 0.00f;
static const int pid_max_yaw = 400;

static float pid_i_mem_roll, pid_i_mem_pitch, pid_i_mem_yaw;		// for I-controller
static float pid_last_roll_d_error, pid_last_pitch_d_error, pid_last_yaw_d_error;  // for D-controller
static float pid_roll_setpoint, pid_pitch_setpoint, pid_yaw_setpoint;

static float pid_error_temp; // only one temp variable for 3 axis error
static float roll_level_adjust, pitch_level_adjust;

float pid_output_roll, pid_output_pitch, pid_output_yaw;

// for flight procedure
uint8_t start = 0;
static bool angle;


void pidCalculate(void) {
	
	// roll
	pid_error_temp = (gyro_x_rate * -1) - pid_roll_setpoint;
	pid_i_mem_roll += pid_i_gain_roll * pid_error_temp;
	if (pid_i_mem_roll > pid_max_roll) pid_i_mem_roll = pid_max_roll;
	else if (pid_i_mem_roll < pid_max_roll * -1) pid_i_mem_roll = pid_max_roll * -1;
	
	pid_output_roll = pid_p_gain_roll * pid_error_temp + pid_i_mem_roll + pid_d_gain_roll * (pid_error_temp - pid_last_roll_d_error);
	if (pid_output_roll > pid_max_roll) pid_output_roll = pid_max_roll;
	else if (pid_output_roll < pid_max_roll * -1) pid_output_roll = pid_max_roll * -1;
	
	pid_last_roll_d_error = pid_error_temp;
	
	// pitch
	pid_error_temp = gyro_y_rate - pid_pitch_setpoint;
	pid_i_mem_pitch += pid_i_gain_pitch * pid_error_temp;
	if (pid_i_mem_pitch > pid_max_pitch) pid_i_mem_pitch = pid_max_pitch;
	else if (pid_i_mem_pitch < pid_max_pitch * -1) pid_i_mem_pitch = pid_max_pitch * -1;
	
	pid_output_pitch = pid_p_gain_pitch * pid_error_temp + pid_i_mem_pitch + pid_d_gain_pitch * (pid_error_temp - pid_last_pitch_d_error);
	if (pid_output_pitch > pid_max_pitch) pid_output_pitch = pid_max_pitch;
	else if (pid_output_pitch < pid_max_pitch * -1) pid_output_pitch = pid_max_pitch * -1;
	
	pid_last_pitch_d_error = pid_error_temp;
	
	// yaw
	pid_error_temp = (gyro_z_rate * -1) - pid_yaw_setpoint;
	pid_i_mem_yaw += pid_i_gain_yaw * pid_error_temp;
	if (pid_i_mem_yaw > pid_max_yaw) pid_i_mem_yaw = pid_max_yaw;
	else if (pid_i_mem_yaw < pid_max_yaw * -1) pid_i_mem_yaw = pid_max_yaw * -1;
	
	pid_output_yaw = pid_p_gain_yaw * pid_error_temp + pid_i_mem_yaw + pid_d_gain_yaw * (pid_error_temp - pid_last_yaw_d_error);
	if (pid_output_yaw > pid_max_yaw) pid_output_yaw = pid_max_yaw;
	else if (pid_output_yaw < pid_max_yaw * -1) pid_output_yaw = pid_max_yaw * -1;
	
	pid_last_yaw_d_error = pid_error_temp;
	
}

void armProcedure(void) {
	
	//pitch_level_adjust = angle_pitch * 15;
	//roll_level_adjust = angle_roll * 15;
	
	pitch_level_adjust = pitchAHRS * 15;
	roll_level_adjust = (rollAHRS * -1.0f) * 15;
	
	angle = (rcValue[6] < 1050) ? true : false;
	//angle = true;
	
	if (!angle) {
		pitch_level_adjust = 0;
		roll_level_adjust = 0;
	}
	
	// To pre-arm: throttle low and yaw left
	if (rcValue[2] < 1050 && rcValue[3] < 1050) 
		start = 1;
	
	if (start == 1 && rcValue[2] < 1050 && rcValue[3] > 1450) {
		start = 2;
		
		LED_ON;							// turn on led
		
		angle_pitch = angle_pitch_acc;			// set the angles to the acc angle 
		angle_roll = angle_roll_acc;				// set the angles to the acc angle
		
		// Reset PID controllers for a smoother start
		pid_i_mem_roll = 0;
    pid_last_roll_d_error = 0;
    pid_i_mem_pitch = 0;
    pid_last_pitch_d_error = 0;
    pid_i_mem_yaw = 0;
    pid_last_yaw_d_error = 0;
	}
	
	// to stop the motors: throttle low and yaw right
	if (start == 2 && rcValue[2] < 1050 && rcValue[3] > 1950) {
		start = 0;
		LED_OFF;
	}
	
	// roll rates calculation
	pid_roll_setpoint = 0;
	if (rcValue[0] > 1508) pid_roll_setpoint = rcValue[0] - 1508;
	else if (rcValue[0] < 1492) pid_roll_setpoint = rcValue[0] - 1492;
	
	pid_roll_setpoint -= roll_level_adjust;
	pid_roll_setpoint /= ROLL_RATE;
	
	// pitch rates calculation
	pid_pitch_setpoint = 0;
	if (rcValue[1] > 1508) pid_pitch_setpoint = rcValue[1] - 1508;
	else if (rcValue[1] < 1492) pid_pitch_setpoint = rcValue[1] - 1492;
	
	pid_pitch_setpoint -= pitch_level_adjust;
	pid_pitch_setpoint /= PITCH_RATE;
	
	// yaw rates calculation
	pid_yaw_setpoint = 0;
	if (rcValue[2] > 1050) {	// stop yaw when turning off the motors
		if (rcValue[3] > 1508) pid_yaw_setpoint = (rcValue[3] - 1508) / YAW_RATE;
		else if (rcValue[3] < 1492) pid_yaw_setpoint = (rcValue[3] - 1492) / YAW_RATE;
	}

}
