#include "image.h"

#define DOWNSAMPLE_S 100
#define DOWNSAMPLE_C 5
#define IMAGE_HEIGHT 64
#define DOWNSAMPLE_X 10
#define DOWNSAMPLE_Y 8
#define SEARCH_LEFT 10
#define SEARCH_RIGHT 117

uint8 threshold = 0;

uint8 maxl = 0;
uint8 maxlpx = 0;

uint8 lEdge[64];
uint8 rEdge[64];
uint8 roadMid[64];
uint8 roadWidth[64];

uint8 rowWeight[64];

void image_process()
{
    downsample(mt9v03x_image);
    // downsample(imagea);
    if (doBin)
        binarize();
    if (doCal)
    {
        getLongestWhiteline();
        findEdge();
        getError();
    }
    // fsmJudge();
    // image_err_calculate();
}

uint8 otsu()
{
    uint8 tvalue;
    uint16 histogram[DOWNSAMPLE_S] = {0};
    uint32 histstd[DOWNSAMPLE_S] = {0};

    // 直方图统计
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

    // 初始化积分表
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

    // 遍历计算类间方差
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

    // 寻找方差最小的灰度级，如果有多个则取平均
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

    // 防止无效结果
    if (thres)
    {
        // 首次进入更新记录阈值
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
        roadMid[i] = (lEdge[i] + rEdge[i]) / 2;
        roadWidth[i] = rEdge[i] - lEdge[i];
    }
}

void getError()
{
    error = 0;
    uint16 weightSum = 0;
    for (int i = prospectU; i < prospectL; i++)
    {
        error += (roadMid[i] - IMAGEMIDLINE) * i;
        weightSum += i;
    }
    error /= weightSum;
}