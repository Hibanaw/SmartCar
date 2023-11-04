#ifndef _PARAM_H_
#define _PARAM_H_

#include "zf_common_headfile.h"

#define IMAGEMIDLINE 64

extern uint16 g_button;

extern uint8 thresMax;
extern uint8 thresMin;

extern uint8 doBin;
extern uint8 calThres;
extern uint8 doCal;

extern uint8 prospectL;
extern uint8 prospectU;

extern int32 error;


extern int32 servoKp;
extern int32 servoKi;
extern int32 servoKd;

extern int32 output;


extern uint8 doMotor;
extern uint8 doSteer;

extern uint16 duty;

extern const uint16 image_data[];
extern const uint8 imagea[120][188];
#endif 
