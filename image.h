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
void fsmJudge();
void edgeFix();
void findPath();
void getSteerError();

void _drawLine(int x1, int y1, int x2, int y2, uint8 array[])

#endif