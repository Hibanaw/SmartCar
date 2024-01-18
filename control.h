#ifndef _CONTROL_H_
#define _CONTROL_H_

#include "zf_common/zf_common_typedef.h"

extern int32 lMotorOutput;
extern int32 rMotorOutput;

void control();
void calcMotorTargrt();
void motorControl();
void steerControl();

#endif