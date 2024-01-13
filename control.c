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
    static float lError[3];
    static float rError[3];
    static int32 lOutput;
    static int32 rOutput;
    int32 lPreOuput;
    int32 lOutputP;
    int32 lOutputI;
    int32 lOutputD;

    int32 rPreOuput;
    int32 rOutputP;
    int32 rOutputI;
    int32 rOutputD;

    lPreOuput = lOutput;
    lError[2] = lError[1];
    lError[1] = lError[0];
    lError[0] = speedTarget - lSpeed;
    lOutputP = motorKp * (lError[0] - lError[1]);
    lOutputI = motorKi * lError[0];
    lOutputD = motorKd * (lError[0]-2*lError[1]+lError[2]);
    lOutput = lOutputP + lOutputI + lOutputD + lPreOuput;

    rPreOuput = rOutput;
    rError[2] = rError[1];
    rError[1] = rError[0];
    rError[0] = speedTarget - rSpeed;
    rOutputP = motorKp * (rError[0] - rError[1]);
    rOutputI = motorKi * rError[0];
    rOutputD = motorKd * (rError[0]-2*rError[1]+rError[2]);
    rOutput = lOutputP + lOutputI + lOutputD + lPreOuput;

    motorOutput(lOutput, rOutput);
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
