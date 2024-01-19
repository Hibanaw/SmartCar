#include "servo.h"

#include "utils.h"
#include "definition.h"

void motorInit()
{
    pwm_init(PIN_MOTORL_A, 17000, 0);
    pwm_init(PIN_MOTORL_B, 17000, 0);
    pwm_init(PIN_MOTORR_A, 17000, 0);
    pwm_init(PIN_MOTORR_B, 17000, 0);
}

void motorOutput(int32 lDuty, int32 rDuty)
{
    lDuty = MAX(MIN(PWM_DUTY_MAX * 0.7, lDuty), 0);
    rDuty = MAX(MIN(PWM_DUTY_MAX * 0.7, rDuty), 0);
    // int32 lHalfDuty = lDuty / 2;
    // int32 rHalfDuty = rDuty / 2;
    // pwm_set_duty(PIN_MOTORL_A, HALF_MAXDUTY - lHalfDuty);
    // pwm_set_duty(PIN_MOTORL_B, HALF_MAXDUTY + lHalfDuty);
    // pwm_set_duty(PIN_MOTORR_A, HALF_MAXDUTY - rHalfDuty);
    // pwm_set_duty(PIN_MOTORR_B, HALF_MAXDUTY + rHalfDuty);
    
    pwm_set_duty(PIN_MOTORL_A,  lDuty);
    pwm_set_duty(PIN_MOTORR_A,  rDuty);

}

void steerInit()
{
    pwm_init(PIN_STEER, 50, STEER_MID);
}

void steerOutput(int32 output)
{
    uint32 duty = STEER_MID - output;
    if (duty < STEER_MID - STEER_RANGE)
        duty = STEER_MID - STEER_RANGE;
    if (duty > STEER_MID + STEER_RANGE)
        duty = STEER_MID + STEER_RANGE;
    pwm_set_duty(PIN_STEER, duty);
}
