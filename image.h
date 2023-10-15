#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "zf_common_headfile.h"

#include "param.h"


extern uint8 image[64][128];
extern uint8 whiteLineLength[128];
extern uint8 whiteLineStart[128];
extern uint8 threshold;
extern uint8 maxl;
extern uint8 maxlpx;
extern uint8 lEdge[64];
extern uint8 rEdge[64];
extern uint8 roadMid[64];

void image_process();
uint8 otsu();
void downsample(const uint8 image_ori[MT9V03X_H][MT9V03X_W]);
void binarize();
void getLongestWhiteline();
void findEdge();
void judge_useful_line();
void get_edge_center_useline();
void Correlation_get();
void image_err_calculate();
float limiter(float value, float min, float max);

#endif