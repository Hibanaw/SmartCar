#include "image.h"
#include "zf_common_headfile.h"

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

uint8 threshold = 0;

uint8 maxl = 0;
uint8 maxlpx = 0;

uint8 lEdge[64];
uint8 rEdge[64];
uint8 roadMid[64];
uint8 roadWidth[64];

uint8 rowWeight[64];

struct FeaturePoints fPoints;

enum RouteState state;

void image_process()
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
        edgeFix();
        findPath();
        getSteerError();
    }
}

void findFeaturePoints()
{
    memset(&fPoints, 0, sizeof(fPoints));
    fPoints.Ay = fPoints.By = 63;
    fPoints.Ax = lEdge[fPoints.Ay];
    fPoints.Bx = rEdge[fPoints.By];
    float32 pdl, pdr;
    for (int i = 63; i > 63 - maxl; i--)
    {
        float32 dl, dr;
        if (i <= 63 - DIFF_DT && i >= DIFF_DT)
        {
            dl = lEdge[i - DIFF_DT] - lEdge[i + DIFF_DT] / (2 * DIFF_DT);
            dr = rEdge[i - DIFF_DT] - rEdge[i + DIFF_DT] / (2 * DIFF_DT);
            if (i > DIFF_DT)
            {
                if (abs(atan(dl) - atan(pdl)) > DIFF_THRESHOLD)
                {
                    fPoints.Gx = lEdge[i];
                    fPoints.Gy = i;
                }
                if (abs(atan(dr) - atan(pdr)) > DIFF_THRESHOLD)
                {
                    fPoints.Hx = rEdge[i];
                    fPoints.Hy = i;
                }
            }
            pdl = dl;
            pdr = dr;
        }
        if (!fPoints.Cy && lEdge[i - 1] < lEdge[i])
        {
            fPoints.Cy = i;
            fPoints.Cx = lEdge[i];
        }
        if (fPoints.Cy && lEdge[i - 1] > fPoints.Cx)
        {
            fPoints.Ey = i;
            fPoints.Ex = lEdge[i];
        }
        if (!fPoints.Dy && rEdge[i - 1] < rEdge[i])
        {
            fPoints.Dy = i;
            fPoints.Dx = rEdge[i];
        }
        if (fPoints.Dy && rEdge[i - 1] > fPoints.Dx)
        {
            fPoints.Fy = i;
            fPoints.Fx = rEdge[i];
        }
    }
}

void fsmJudge()
{
    // ��ʻ��ֱ��
    if (state == STRAIGHT)
    {
        // ʮ��
        if (fPoints.Cy != fPoints.Ay && fPoints.Ey != fPoints.Ay && fPoints.Dy != fPoints.By && fPoints.Fy != fPoints.By)
        {
            state = CROSS;
            return;
        }

        // ����
        if (fPoints.Cy != fPoints.Ay && fPoints.Ey != fPoints.Ay && fPoints.By == fPoints.Dy == fPoints.Fy)
        {
            state = LEFT_ROUNDABOUT_IN;
            return;
        }
        if (fPoints.Dy != fPoints.By && fPoints.Fy != fPoints.By && fPoints.Ay == fPoints.Cy == fPoints.Ey)
        {
            state = RIGHT_ROUNDABOUT_IN;
            return;
        }
    }

    // �뻷��
    if (state == LEFT_ROUNDABOUT_IN)
    {
        if (fPoints.Ay == fPoints.By == fPoints.Cy == fPoints.Dy == fPoints.Ey == fPoints.Fy)
        {
            state = LEFT_ROUNDABOUT_KEEP;
        }
        return;
    }
    if (state == RIGHT_ROUNDABOUT_IN)
    {
        if (fPoints.Ay == fPoints.By == fPoints.Cy == fPoints.Dy == fPoints.Ey == fPoints.Fy)
        {
            state = RIGHT_ROUNDABOUT_KEEP;
        }
        return;
    }
    // ������
    if (state == LEFT_ROUNDABOUT_KEEP)
    {
        if (fPoints.Cy != fPoints.Ay && fPoints.Ey != fPoints.Ay && fPoints.Dy != fPoints.By && fPoints.Fy != fPoints.By)
        {
            state = LEFT_ROUNDABOUT_OUT;
        }
        return;
    }
    if (state == RIGHT_ROUNDABOUT_KEEP)
    {
        if (fPoints.Cy != fPoints.Ay && fPoints.Ey != fPoints.Ay && fPoints.Dy != fPoints.By && fPoints.Fy != fPoints.By)
        {
            state = RIGHT_ROUNDABOUT_OUT;
        }
        return;
    }

    // ����
    if (state == STRAIGHT || state == UPHILL)
    {
        if (fPoints.Gy != fPoints.Ay && fPoints.Hy != fPoints.By)
        {
            state = UPHILL;
            return;
        }
        if (fPoints.Ay == fPoints.By == fPoints.Cy == fPoints.Dy == fPoints.Ey == fPoints.Fy)
        {
            state = STRAIGHT;
        }
    }

    // // ֱ��
    // if (fPoints.Ay == fPoints.By == fPoints.Cy == fPoints.Dy == fPoints.Ey == fPoints.Fy)
    // {
    //     state = STRAIGHT;
    //     return;
    // }

    // �������������ֱ�ߴ���
    state = STRAIGHT;
}

void edgeFix()
{
    switch (state)
    {
    case CROSS:
        for (int i = fPoints.Cy; i >= fPoints.Ey; i--)
        {
            _drawLine(fPoints.Cx, fPoints.Cy, fPoints.Ex, fPoints.Ey, lEdge);
            _drawLine(fPoints.Dx, fPoints.Dy, fPoints.Fx, fPoints.Fy, rEdge);
        }
        break;
    default:
        break;
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
        y2 = tmpy;
        y1 = y2;
        tmpx = x1;
        x2 = tmpx;
        x1 = x2;
    }
    for (int i = y1; i < y2; i++)
    {
        array[i] = x1 + (float32)(x2 - x1) / (y2 - y1);
    }
}