#include "control.h"

#include "zf_common_headfile.h"

#include "encoder.h"
#include "param.h"
#include "servo.h"
#include "image.h"
#include "utils.h"

int32 lMotorOutput;
int32 rMotorOutput;

enum SteerState steerState = STEER_PID;

void control()
{
    if (doMotor)
    {
        calcMotorTargrt();
        motorControl();
    }
    if (doSteer)
    {
        forceSteer();
        steerControl();
    }
}

void calcMotorTargrt()
{
    switch (routeState)
    {
    case UPHILL:
        speedTarget = speed * 1.2;
        break;
    case STOP:
        speedTarget = 0;
        break;
    case STRAIGHT:
        speedTarget = speed;
        break;
    case PARK_OUT:
    case LEFT_ROUNDABOUT_IN_1:
    case RIGHT_ROUNDABOUT_IN_1:
    case LEFT_ROUNDABOUT_OUT:
    case RIGHT_ROUNDABOUT_OUT:
    case CROSS:
        speedTarget = 0.5 * speed;
        break;
        
    case PARK_IN:
        speedTarget = 0.3 * speed;
        break;
    default:
        speedTarget = speed;
        break;
    }
    speedTarget /= 100;
}



void motorControl()
{
    static float lError[3];
    static float rError[3];
    int32 lPreOuput;
    int32 lOutputP;
    int32 lOutputI;
    int32 lOutputD;

    int32 rPreOuput;
    int32 rOutputP;
    int32 rOutputI;
    int32 rOutputD;

    lPreOuput = lMotorOutput;
    lError[2] = lError[1];
    lError[1] = lError[0];
    lError[0] = speedTarget - lSpeed;
    lOutputP = motorKp * (lError[0] - lError[1]);
    lOutputI = motorKi * lError[0];
    lOutputD = motorKd * (lError[0] - 2 * lError[1] + lError[2]);
    lMotorOutput = lOutputP + lOutputI + lOutputD + lPreOuput;
    lMotorOutput = MAX(MIN(5000, lMotorOutput), 0);
    
    rPreOuput = rMotorOutput;
    rError[2] = rError[1];
    rError[1] = rError[0];
    rError[0] = speedTarget - rSpeed;
    rOutputP = motorKp * (rError[0] - rError[1]);
    rOutputI = motorKi * rError[0];
    rOutputD = motorKd * (rError[0] - 2 * rError[1] + rError[2]);
    rMotorOutput = rOutputP + rOutputI + rOutputD + rPreOuput;
    rMotorOutput = MAX(MIN(5000, rMotorOutput), 0);
    if(speedTarget == 0) lMotorOutput = rMotorOutput = 0;
    motorOutput(lMotorOutput, rMotorOutput);
}



void forceSteer(){
    switch (routeState)
    {
    case PARK_OUT:
    case PARK_IN:
        if(!isAntiClockWise)
            steerState = STEER_RIGHT;
        else 
            steerState = STEER_LEFT;
        break;
    case LEFT_ROUNDABOUT_OUT:
    case LEFT_ROUNDABOUT_IN_1:
        steerState = STEER_LEFT;
        break;
    case RIGHT_ROUNDABOUT_OUT:
    case RIGHT_ROUNDABOUT_IN_1:
        steerState = STEER_RIGHT;
        break;
    default:
        steerState = STEER_PID;
        break;
    }
}

void steerControl()
{
    static int32 preError;
    static int32 outputI;
    int32 outputP;
    int32 outputD;
    int32 output = 0;
    switch (steerState)
    {
    case STEER_LEFT:
        steerTarget = -STEER_RANGE + 10;
        break;
    case STEER_RIGHT:
        steerTarget = STEER_RANGE - 10;
        break;
    default:
        outputP = steerError * steerKp / 1000;
        outputI += steerError * steerKi / 1000;
        outputD = (steerError - preError) * steerKd / 1000;
        output = outputP + outputI + outputD;
        preError = steerError;
        steerTarget = output;
        break;
    }
    
    steerOutput(steerTarget);
}
