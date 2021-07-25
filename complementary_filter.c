#include "stm32f411xe.h"
#include "stm32f4xx.h"
#include "global.h"
#include <math.h>
#include <stdlib.h>

#include "complementary_filter.h"
#include "mpu6000.h"

float angle_pitch_acc, angle_roll_acc;
float gyro_pitch_rate, gyro_roll_rate, gyro_yaw_rate;
float gyro_x_rate, gyro_y_rate, gyro_z_rate;
float angle_pitch, angle_roll;

void getAccAngles(void) {
	
	// calculate total accelerometer vector
	float acc_total_vector = sqrt((acc_pitch * acc_pitch) + (acc_roll * acc_roll) + (acc_yaw * acc_yaw));
	
	// prevent the asin function to produce a NaN
	if(abs(acc_pitch) < acc_total_vector) 
		angle_pitch_acc = asinf(acc_pitch / acc_total_vector) * (1.0f / (M_PI / 180.0f));
	
	// prevent the asin function to produce a NaN
	if(abs(acc_roll) < acc_total_vector)
		angle_roll_acc = asinf(acc_roll / acc_total_vector) * (1.0f / (M_PI / 180.0f));
		
}

void getGyroRates(void) {
	
	#ifdef USE_COMP_FILTER
	// Gyro 500DPS full scale -> output 65.5 when the rotational rate is 1deg/sec.
	gyro_pitch_rate = (gyro_pitch_rate * 0.7f) + ((gyro_pitch / 65.5f) * 0.3f);
	gyro_roll_rate = (gyro_roll_rate * 0.7f) + ((gyro_roll / 65.5f) * 0.3f);
	gyro_yaw_rate = (gyro_yaw_rate * 0.7f) + ((gyro_yaw  / 65.5f) * 0.3f);
	#endif
	
	// Gyro 500DPS full scale -> output 65.5 when the rotational rate is 1deg/sec.
	gyro_x_rate = (gyro_x_rate * 0.7f) + ((gyro_x / 65.5f) * 0.3f);
	gyro_y_rate = (gyro_y_rate * 0.7f) + ((gyro_y / 65.5f) * 0.3f);
	gyro_z_rate = (gyro_z_rate * 0.7f) + ((gyro_z  / 65.5f) * 0.3f);
	
}

void complementaryCalculate(void) {
	
	// integrating the gyro output to calculate the total travelled angle in one looptime
	angle_pitch += (gyro_pitch * (1.0f / REFRESH_RATE)) / 65.5f;
	angle_roll += (gyro_roll * (1.0f / REFRESH_RATE)) / 65.5f;
	
	// transfer the angles if yaw movement is induced
	angle_pitch -= angle_roll * sinf(((gyro_yaw * (1.0f / REFRESH_RATE)) / 65.5f) * (M_PI / 180.0f));
	angle_roll += angle_pitch * sinf(((gyro_yaw * (1.0f / REFRESH_RATE)) / 65.5f) * (M_PI / 180.0f));
	
	// get the accelerometer calculated angles
	getAccAngles();
	
	// complement both gyroscope and accelerometer angle readings
	angle_pitch = angle_pitch * 0.9996f + angle_pitch_acc * 0.0004f;
	angle_roll = angle_roll * 0.9996f + angle_roll_acc * 0.0004f;
	
}

