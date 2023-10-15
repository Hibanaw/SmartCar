#include "pwm.h"

pwm_channel_enum channel_list[CHANNEL_NUMBER] = {PWM_CH1, PWM_CH2, PWM_CH3, PWM_CH4};

void pwmInit(){
    pwm_init(PWM_CH1, 17000, 0);
    pwm_init(PWM_CH2, 17000, 0);
    pwm_init(PWM_CH3, 17000, 0);
    pwm_init(PWM_CH4, 17000, 0);    
}

void pwmUpdate(){
    for(int i = 0; i < CHANNEL_NUMBER; i++){
        pwm_set_duty(channel_list[i], duty[i]);
    }
}