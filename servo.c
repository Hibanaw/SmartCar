#include "servo.h"

void motorInit(){
    pwm_init(MOTORL_A, 17000, 0);
    pwm_init(MOTORL_B, 17000, 0);
    pwm_init(MOTORR_A, 17000, 0);
    pwm_init(MOTORR_B, 17000, 0);
}

void motorOutput(uint32 lDuty, uint32 rDuty){
    uint32 lHalfDuty = lDuty/2;
    uint32 rHalfDuty = rDuty/2;
    pwm_set_duty(MOTORL_A, HALF_MAXDUTY - lHalfDuty);
    pwm_set_duty(MOTORL_B, HALF_MAXDUTY + lHalfDuty);
    pwm_set_duty(MOTORR_A, HALF_MAXDUTY - rHalfDuty);
    pwm_set_duty(MOTORR_B, HALF_MAXDUTY + rHalfDuty);
}

void steerInit(){
    pwm_init(STEER, 50, STEER_MID);
}


void steerOutput(int32 angle){
    uint32 duty = STEER_MID - angle;
    if (duty < STEER_MID - STEER_RANGE) duty = STEER_MID - STEER_RANGE;
    if (duty > STEER_MID + STEER_RANGE) duty = STEER_MID + STEER_RANGE;    
    pwm_set_duty(STEER, duty);
}