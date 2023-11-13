#ifndef _SERVO_H_
#define _SERVO_H_


#include "zf_common_headfile.h"

#define HALF_MAXDUTY (PWM_DUTY_MAX/2)



#define STEER_MID (530)
#define STEER_RANGE (70)


void motorInit();
void steerInit();
void motorOutput(uint32 lDuty, uint32 rDuty);
void steerOutput(int32 output);
#endif