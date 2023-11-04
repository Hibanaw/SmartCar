#ifndef _SERVO_H_
#define _SERVO_H_


#include "zf_common_headfile.h"

#define HALF_MAXDUTY (PWM_DUTY_MAX/2)

#define MOTORL_A (TIM5_PWM_CH1_A0)
#define MOTORL_B (TIM5_PWM_CH3_A2)
#define MOTORR_A (TIM5_PWM_CH2_A1)
#define MOTORR_B (TIM5_PWM_CH4_A3)

#define STEER (TIM2_PWM_CH1_A15)

#define STEER_MID (530)
#define STEER_RANGE (70)


void motorInit();
void steerInit();
void motorOutput(uint32 lDuty, uint32 rDuty);
void steerOutput(int32 angle);
#endif