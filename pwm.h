#ifndef _PWM_H_
#define _PWM_H_

#include "zf_common_headfile.h"

#include "param.h"

#define CHANNEL_NUMBER          (4)

#define PWM_CH1                 (TIM5_PWM_CH1_A0)
#define PWM_CH2                 (TIM5_PWM_CH2_A1)
#define PWM_CH3                 (TIM5_PWM_CH3_A2)
#define PWM_CH4                 (TIM5_PWM_CH4_A3)

void pwmInit();
void pwmUpdate();

#endif 