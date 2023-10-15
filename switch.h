#ifndef _SWITCH_H_
#define _SWITCH_H_

#include "zf_common_headfile.h"

#include "param.h"

extern uint8 switch1;
extern uint8 switch2;

#define SWITCH1                 (D14)
#define SWITCH2                 (D15)

void switchInit();
void switchScan();

#endif