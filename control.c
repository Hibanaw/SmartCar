#include "control.h"

#include "zf_common_headfile.h"

#include "encoder.h"
#include "param.h"
#include "servo.h"

void control(){
    if(doMotor){
        motorControl();
    }
    if(doSteer){
        steerControl();
    }
}

void motorControl(){
    float speed = (lSpeed + rSpeed) / 2;
    static float error[3];
    static int32 output;
    int32 preOuput;
    int32 outputP;
    int32 outputI;
    int32 outputD;
    preOuput = output;
    error[2] = error[1];
    error[1] = error[0];
    error[0] = speedTarget - speed;
    outputP = motorKp * (error[0] - error[1]);
    outputI = motorKi * error[0];
    outputD = motorKd * (error[0]-2*error[1]+error[2]);
    output = outputP + outputI + outputD + preOuput;
    motorOutput(output, output);
}


void steerControl()
{
    static int32 preError;
    static int32 outputI;
    int32 outputP;
    int32 outputD;
    int32 output = 0;
    outputP = steerError * steerKp / 1000;
    outputI += steerError * steerKi / 1000;
    outputD = (steerError - preError) * steerKd / 1000;
    output = outputP + outputI + outputD;
    preError = steerError;
    steerTarget = output;
    steerOutput(output);
}
