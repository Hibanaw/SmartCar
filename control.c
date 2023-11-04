#include "control.h"


void control(){
    if(doMotor){
        motorOutput(duty, duty);
    }
    if(doSteer){
        pidControl();
    }
}

void pidControl()
{
    static int32 preError;
    static int32 outputI;
    int32 outputP;
    int32 outputD;
    output = 0;
    outputP = error * servoKp / 1000;
    outputI += error * servoKi / 1000;
    outputD = (error - preError) * servoKd / 1000;
    output = outputP + outputI + outputD;
    preError = error;
    steerOutput(output);
}
