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

extern struct FeaturePoints
{
    // ��ϵ�
    int8 Ax, Ay, Bx, By, Cx, Cy, Dx, Dy, Ex, Ey, Fx, Fy;
} fPoints;

extern enum RouteState
{
    PARK_IN,
    PARK_OUT,
    STOP,
    STRAIGHT,
    LEFT_ROUNDABOUT_IN_0,
    LEFT_ROUNDABOUT_IN_1,
    LEFT_ROUNDABOUT_KEEP,
    LEFT_ROUNDABOUT_OUT,
    RIGHT_ROUNDABOUT_IN_0,
    RIGHT_ROUNDABOUT_IN_1,
    RIGHT_ROUNDABOUT_KEEP,
    RIGHT_ROUNDABOUT_OUT,
    CROSS,
    UPHILL,
} routeState;

extern uint32 stateTime;

void imageProcess();
uint8 otsu();
void downsample(const uint8 image_ori[MT9V03X_H][MT9V03X_W]);
void binarize();
void getLongestWhiteline();
void findEdge();
void fsmJudge();
void edgeFix();
void findPath();
void getSteerError();
void findFeaturePoints();

void _drawLine(int x1, int y1, int x2, int y2, uint8 array[]);
void _drawArc(int x1, int y1, int x2, int y2, uint8 array[]);
#endif
