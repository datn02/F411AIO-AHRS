#ifndef _COMP_FILTER_H_
#define _COMP_FILTER_H_



#define M_PI 3.14159265359f

extern float angle_pitch_acc, angle_roll_acc;
extern float gyro_pitch_rate, gyro_roll_rate, gyro_yaw_rate;
extern float gyro_x_rate, gyro_y_rate, gyro_z_rate;
extern float angle_pitch, angle_roll;

void getAccAngles(void);
void getGyroRates(void);
void complementaryCalculate(void);

#endif
