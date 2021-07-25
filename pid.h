#ifndef _PID_H_
#define _PID_H_


#include <stdint.h>
//extern float pid_i_mem_roll, pid_i_mem_pitch, pid_i_mem_yaw;		// for I-controller
//extern float pid_last_roll_d_error, pid_last_pitch_d_error, pid_last_yaw_d_error;  // for D-controller
extern float pid_output_roll, pid_output_pitch, pid_output_yaw;

extern uint8_t start;


void pidCalculate(void);
void armProcedure(void);

#endif
