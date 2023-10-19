#ifndef _PARAM_H_
#define _PARAM_H_

#include "zf_common_headfile.h"

#define IMAGEMIDLINE 64

extern uint16 g_button;
extern uint16 duty[];


extern uint8 thresMax;
extern uint8 thresMin;

extern uint8 doBin;
extern uint8 calThres;
extern uint8 doCal;

extern uint8 prospectL;
extern uint8 prospectU;

extern int32 error;

extern const uint16 image_data[];
extern const uint8 imagea[120][188];
#endif 
