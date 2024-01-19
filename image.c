#include "image.h"
#include "zf_common_headfile.h"
#include "utils.h"

#define DOWNSAMPLE_S (100)
#define DOWNSAMPLE_C (5)
#define IMAGE_HEIGHT (64)
#define DOWNSAMPLE_X (10)
#define DOWNSAMPLE_Y (8)
#define SEARCH_LEFT (10)
#define SEARCH_RIGHT (117)

#define DIFF_DT (1)
#define DIFF_THRESHOLD (ANGLE_TO_RAD(10))

uint8 image[64][128];

uint8 threshold = 165;

uint8 maxl = 0;
uint8 maxlpx = 0;

uint8 lEdge[64];
uint8 rEdge[64];
uint8 roadMid[64];
uint8 roadWidth[64];

uint8 rowWeight[64];

struct FeaturePoints fPoints;

uint32 stateTime;

enum RouteState routeState;

void imageProcess()
{
    downsample(mt9v03x_image);
    if (doBin)
        binarize();
    if (doCal)
    {
        getLongestWhiteline();
        findEdge();
        findFeaturePoints();
        fsmJudge();
        // if(doEdgeFix)
        edgeFix();
        findPath();
        getSteerError();
    }
}

void findFeaturePoints()
{
    memset(&fPoints, 0, sizeof(fPoints));
    fPoints.Ay = fPoints.By = prospectL;
    fPoints.Ax = lEdge[fPoints.Ay];
    fPoints.Bx = rEdge[fPoints.By];
    if(!fPoints.Ax||!fPoints.Bx) return;
    for (int i = prospectL; i > MAX(prospectU, 63 - maxl) + 5; i--)
    {
        if (!fPoints.Cy && lEdge[i - 1] < lEdge[i]-20)
        {
            fPoints.Cy = i;
            fPoints.Cx = lEdge[i];
            i -= 20;
        }
        if (fPoints.Cy && lEdge[i] >= fPoints.Cx + (fPoints.Cx - fPoints.Ax) * (fPoints.Cy - i) / (fPoints.Ay - fPoints.Cy))
        {
            fPoints.Ey = i;
            fPoints.Ex = lEdge[i];
            break;
        }
    }
    for (int i = prospectL; i > MAX(prospectU, 63 - maxl) + 5; i--)
    {
        if (!fPoints.Dy && rEdge[i - 1] > rEdge[i]+20)
        {
            fPoints.Dy = i;
            fPoints.Dx = rEdge[i];
            i -= 20;
        }
        if (fPoints.Dy && rEdge[i] <= fPoints.Dx + (fPoints.Dx - fPoints.Bx) * (fPoints.Dy - i) / (fPoints.By - fPoints.Dy))
        {
            fPoints.Fy = i;
            fPoints.Fx = rEdge[i];
            break;
        }
    }
}

void fsmJudge()
{
    uint32 time = system_getval_ms();
    if (routeState == PARK_OUT)
    {
        if (time - stateTime < 500)
        {
            return;
        }
        routeState = STRAIGHT;
        stateTime = time;
    }
    if (routeState == PARK_IN)
    {
        if (time - stateTime < 700)
        {
            return;
        }
        routeState = STRAIGHT;
        stateTime = time;
    }
    // 车库
    for (int i = 16, c = 0; i < 112; i++)
    {
        if (c >= 10)
        {
            routeState = PARK_IN;
            stateTime = time;
            return;
        }
        if (image[prospectL-10][i] != image[prospectL-10][i - 1])
        {
            c++;
        }
    }
    // 停车
    for (int i = 0, c = 0; i < 128; i++)
    {
        if (c > 100)
        {
            routeState = STOP;
            stateTime = time;
            return;
        }
        if (image[prospectL][i] == 0)
        {
            c++;
        }
    }
    // 行驶在直线
    if (routeState == STRAIGHT)
    {
        // 环岛
        if (fPoints.Cy && fPoints.Ey && !fPoints.Dy && !fPoints.Fy)
        {
            routeState = LEFT_ROUNDABOUT_IN_0;
            stateTime = time;
            return;
        }
        if (fPoints.Dy && fPoints.Fy && !fPoints.Cy && !fPoints.Ey)
        {
            routeState = RIGHT_ROUNDABOUT_IN_0;
            stateTime = time;
            return;
        }
    }

    // 十字
    if (routeState == STRAIGHT || routeState == CROSS)
    {
        if (fPoints.Dy && fPoints.Fy && fPoints.Cy && fPoints.Ey)
        {
            routeState = CROSS;
            stateTime = time;
            return;
        }
    }

    // 环岛状态2
    if (routeState == LEFT_ROUNDABOUT_IN_0)
    {
        if (time - stateTime < 800)
            return;
        if (fPoints.Cy && fPoints.Ey && !fPoints.Dy && !fPoints.Fy)
        {
            routeState = LEFT_ROUNDABOUT_IN_1;
            stateTime = time;
            return;
        }
    }
    if (routeState == RIGHT_ROUNDABOUT_IN_0)
    {
        if (time - stateTime < 800)
            return;
        if (fPoints.Dy && fPoints.Fy && !fPoints.Cy && !fPoints.Ey)
        {
            routeState = RIGHT_ROUNDABOUT_IN_1;
            stateTime = time;
            return;
        }
    }
    // 入环岛
    if (routeState == LEFT_ROUNDABOUT_IN_1)
    {
        if (time - stateTime < 1000)
            return;
        if (!fPoints.Cy && fPoints.Dy && fPoints.Fy)
        {
            routeState = LEFT_ROUNDABOUT_KEEP;
            stateTime = time;
        }
        else if (time - stateTime > 2000)
            routeState = STRAIGHT;
        return;
    }
    if (routeState == RIGHT_ROUNDABOUT_IN_1)
    {
        if (time - stateTime < 1000)
            return;
        if (!fPoints.Dy && fPoints.Cy && fPoints.Ey)
        {
            routeState = RIGHT_ROUNDABOUT_KEEP;
            stateTime = time;
        }
        else if (time - stateTime > 2000)
            routeState = STRAIGHT;
        return;
    }
    // 出环岛
    if (routeState == LEFT_ROUNDABOUT_KEEP)
    {
        if (time - stateTime < 1000)
            return;
        if (fPoints.Cy && fPoints.Dy)
        {
            routeState = LEFT_ROUNDABOUT_OUT;
            stateTime = time;
        }
        else if (time - stateTime > 5000)
        {
            routeState = STRAIGHT;
            stateTime = time;
        }
        return;
    }
    if (routeState == RIGHT_ROUNDABOUT_KEEP)
    {
        if (time - stateTime < 1000)
            return;
        if (fPoints.Cy && fPoints.Dy)
        {
            routeState = RIGHT_ROUNDABOUT_OUT;
            stateTime = time;
        }
        else if (time - stateTime > 5000)
        {
            routeState = STRAIGHT;
            stateTime = time;
        }
        return;
    }

    if (routeState == LEFT_ROUNDABOUT_OUT)
    {
        if (time - stateTime > 500)
        {
            routeState = STRAIGHT;
            stateTime = time;
        }
        return;
    }
    if (routeState == LEFT_ROUNDABOUT_OUT)
    {
        if (time - stateTime > 500)
        {
            routeState = STRAIGHT;
            stateTime = time;
        }
        return;
    }
    // 上坡
    // if (routeState == STRAIGHT)
    {
        if (63 - maxl < prospectL && !fPoints.Cy && !fPoints.Dy)
        {
            int8 kl1, kl2, kr1, kr2;
            kl1 = ((int8)lEdge[prospectL - 5] - lEdge[prospectL]);
            kl2 = ((int8)lEdge[prospectU] - lEdge[prospectU + 5]);
            kr1 = -((int8)rEdge[prospectL - 5] - rEdge[prospectL]);
            kr2 = -((int8)rEdge[prospectU] - rEdge[prospectU + 5]);
            if (kl1 - kl2 > 1 && kr1 - kr2 > 1)
            {
                routeState = UPHILL;
                stateTime = time;
                return;
            }
        }
    }

    if (routeState == UPHILL)
    {
        if (time - stateTime >= 1000)
        {
            routeState = STRAIGHT;
            stateTime = time;
        }
        return;
    }

    // 直线
    if (fPoints.Ay == fPoints.By == fPoints.Cy == fPoints.Dy == fPoints.Ey == fPoints.Fy)
    {
        routeState = STRAIGHT;
        return;
    }

    // 其他所有情况按直线处理
    routeState = STRAIGHT;
}

void edgeFix()
{
    // int fl = 0, fr = 0;
    // int fly = 0, fry = 0;
    // for(int i = 63-maxl; i < 63; i++){
    //     if(lEdge[i] == 0 && rEdge[i] != 127){
    //         fl++;
    //         fly = i;
    //     }
    //     else break;
    // }
    // for(int i = 63-maxl; i < 63; i++){
    //     if(rEdge[i] == 127 && lEdge[i] != 0){
    //         fr++;
    //         fry = i;
    //     }
    //     else break;
    // }
    // for(int i = 63-maxl-fl; i > 63-maxl; i--){
    //     lEdge[i] = rEdge[i] - rEdge[fly];
    // }`
    // for(int i = 63-maxl-fr; i > 63-maxl; i--){
    //     rEdge[i] = lEdge[i] + 127 - lEdge[fly];
    // }
    switch (routeState)
    {
    case CROSS:
        for (int i = fPoints.Cy; i >= fPoints.Ey; i--)
        {
            _drawLine(fPoints.Cx, fPoints.Cy, fPoints.Ex, fPoints.Ey, lEdge);
            _drawLine(fPoints.Dx, fPoints.Dy, fPoints.Fx, fPoints.Fy, rEdge);
        }
        break;
    // case PARK_IN:
    //     if (!isAntiClockWise)
    //     {
    //         goto RIGHT_ROUNDABOUT_IN_1_LABLE;
    //     }
    //     goto LEFT_ROUNDABOUT_IN_1_LABLE;
    //     break;
    // case PARK_OUT:
    //     if (!isAntiClockWise)
    //     {
    //         goto RIGHT_ROUNDABOUT_OUT_LABLE;
    //     }
    //     goto LEFT_ROUNDABOUT_OUT_LABLE;
    //     break;
    case LEFT_ROUNDABOUT_IN_0:
        _drawLine(fPoints.Cx, fPoints.Cy, fPoints.Ex, fPoints.Ey, lEdge);
        maxl = 63 - fPoints.Ey;
        break;
    case RIGHT_ROUNDABOUT_IN_0:
        _drawLine(fPoints.Dx, fPoints.Dy, fPoints.Fx, fPoints.Fy, rEdge);
        maxl = 63 - fPoints.Fy;
        break;
    //     case LEFT_ROUNDABOUT_IN_1:
    //     LEFT_ROUNDABOUT_IN_1_LABLE:
    //         _drawArc(rEdge[fPoints.Cy], fPoints.Cy, fPoints.Ex, fPoints.Ey, rEdge);
    //         break;
    //     case RIGHT_ROUNDABOUT_IN_1:
    //     RIGHT_ROUNDABOUT_IN_1_LABLE:
    //         _drawArc(lEdge[fPoints.Dy], fPoints.Dy, fPoints.Fx, fPoints.Fy, lEdge);
    //         break;
    //     case LEFT_ROUNDABOUT_OUT:
    //     LEFT_ROUNDABOUT_OUT_LABLE:
    //         _drawArc(fPoints.Dx, fPoints.Dy, fPoints.Cx, 63-maxl, rEdge);
    //         break;
    //     case RIGHT_ROUNDABOUT_OUT:
    //     RIGHT_ROUNDABOUT_OUT_LABLE:
    //         _drawArc(fPoints.Cx, fPoints.Cy, fPoints.Dx, 63-maxl, lEdge);
    //         break;
        // default:
        //     break;
    }
}

uint8 otsu()
{
    uint8 tvalue;
    uint16 histogram[DOWNSAMPLE_S] = {0};
    uint32 histstd[DOWNSAMPLE_S] = {0};

    for (uint16 y = 0; y < IMAGE_HEIGHT; y += DOWNSAMPLE_Y)
    {
        for (uint16 x = SEARCH_LEFT; x <= SEARCH_RIGHT; x += DOWNSAMPLE_X)
        {
            histogram[image[y][x] / DOWNSAMPLE_C]++;
        }
    }

    uint16 background_mean = 0;
    uint16 foreground_mean = 0;
    uint32 background_sum = 0;
    uint32 foreground_sum = 0;
    uint16 background_n = 0;
    uint16 foreground_n = 0;
    uint16 i = thresMin / DOWNSAMPLE_C;

    for (uint16 k = 0; k < i; k++)
    {
        background_n += histogram[k];
        background_sum += histogram[k] * k;
    }

    for (uint16 k = i; k < DOWNSAMPLE_S; k++)
    {
        foreground_n += histogram[k];
        foreground_sum += histogram[k] * k;
    }

    for (; i <= thresMax / DOWNSAMPLE_C; i++)
    {
        background_n += histogram[i];
        foreground_n -= histogram[i];

        if (!background_n)
        {
            continue;
        }
        if (!foreground_n)
        {
            break;
        }
        if (!histogram[i])
        {
            histstd[i] = histstd[i - 1];
        }

        background_sum += histogram[i] * i;
        foreground_sum -= histogram[i] * i;

        background_mean = background_sum / background_n;
        foreground_mean = foreground_sum / foreground_n;

        histstd[i] = background_n * foreground_n * (int32)(background_mean - foreground_mean) * (int32)(background_mean - foreground_mean);
    }

    uint32 temp = 0x00;
    uint16 thres = 0;
    uint16 thres_n = 0;

    for (uint16 i = thresMin / DOWNSAMPLE_C; i <= thresMax / DOWNSAMPLE_C; i++)
    {
        if (histstd[i])
        {
            if (histstd[i] > temp)
            {
                temp = histstd[i];
                thres = i;
                thres_n = 1;
            }
            else if (histstd[i] == temp)
            {
                thres += i;
                thres_n++;
            }
        }
    }

    static uint16 flag = 0;
    static float last_thres = 0.0;

    if (thres)
    {
        if (!flag)
        {
            last_thres = thres / (float)thres_n;
            flag = 1;
        }

        last_thres = 0.95 * last_thres + 0.05 * thres / (float)thres_n;

        tvalue = (uint16)(last_thres * DOWNSAMPLE_C);
    }

    return tvalue;
}

void downsample(const uint8 image_ori[MT9V03X_H][MT9V03X_W])
{
    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 128; j++)
        {
            image[i][j] = image_ori[i * MT9V03X_H / 64][j * MT9V03X_W / 128];
        }
    }
}

void binarize()
{
    if (calThres)
        threshold = otsu();
    uint8 thresholdMap[256] = {0};
    if (!doBin)
        return;
    for (int i = 0; i < threshold; i++)
    {
        thresholdMap[i] = 0;
    }
    for (int i = threshold; i < 256; i++)
    {
        thresholdMap[i] = -1;
    }
    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 128; j++)
        {
            image[i][j] = thresholdMap[image[i][j]];
        }
    }
}

void getLongestWhiteline()
{
    uint16 sum;
    uint8 lcount;
    maxl = 0;
    maxlpx = 0;
    for (int i = 0; i < 128; i++)
    {
        for (int j = 63; j >= 0; j--)
        {
            if (!image[j][i])
            {
                if (63 - j > maxl)
                {
                    maxl = 63 - j;
                    sum = i;
                    lcount = 1;
                }
                if (63 - j == maxl)
                {
                    sum += i;
                    lcount++;
                }
                break;
            }
        }
    }
    maxlpx = sum / lcount;
}

void findEdge()
{
    for (int i = 63 - maxl; i < 64; i++)
    {
        for (int j = maxlpx; j >= 0; j--)
        {
            if (!image[i][j] || j == 0)
            {
                lEdge[i] = j;
                break;
            }
        }
        for (int j = maxlpx; j < 128; j++)
        {
            if (!image[i][j] || j == 127)
            {
                rEdge[i] = j;
                break;
            }
        }
    }
}

void findPath()
{
    for (int i = 63 - maxl; i < 64; i++)
    {
        roadMid[i] = (lEdge[i] + rEdge[i]) / 2;
        roadWidth[i] = rEdge[i] - lEdge[i];
    }
}

void getSteerError()
{
    steerError = 0;
    uint16 weightSum = 0;
    for (int i = 63 - maxl; i < prospectL; i++)
    {
        steerError += (roadMid[i] - IMAGEMIDLINE) * i;
        weightSum += i;
    }
    steerError /= weightSum;
}

void _drawLine(int x1, int y1, int x2, int y2, uint8 array[])
{
    if (y1 > y2)
    {
        int tmpx, tmpy;
        tmpy = y1;
        y1 = y2;
        y2 = tmpy;
        tmpx = x1;
        x1 = x2;
        x2 = tmpx;
    }
    for (int i = y1; i < y2; i++)
    {
        array[i] = x1 + ((float32)(x2 - x1)) / (y2 - y1) * (i - y1);
    }
}

void _drawArc(int x1, int y1, int x2, int y2, uint8 array[])
{
    int x = x2, y = y1;
    int rx = x1 - x, ry = y2 - y;
    for (int i = y; i > y2; i--)
    {
        array[i] = x - (double)rx / ry * sqrt(pow(ry, 2) - pow(i - y, 2));
    }
}
