#ifndef _CONTROL_H_
#define _CONTROL_H_

#include "zf_common/zf_common_typedef.h"

extern int32 lMotorOutput;
extern int32 rMotorOutput;

extern enum SteerState { STEER_LEFT,
                         STEER_RIGHT,
                         STEER_PID } steerState;

void control();
void calcMotorTargrt();
void motorControl();
void forceSteer();
void steerControl();

#endif