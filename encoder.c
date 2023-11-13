#include "encoder.h"

#include <zf_common_headfile.h>

uint16 lEncoderCount;
uint16 rEncoderCount;


float rSpeed;
float lSpeed;
float distance;
// int32 rSpeed_NUMBER;
// int32 lSpeed_NUMBER;

void encoderInit()
{
    encoder_quad_init(TIM3_ENCODER, TIM3_ENCODER_CH1_P02_6, TIM3_ENCODER_CH2_P02_7);
    encoder_quad_init(TIM4_ENCODER, TIM4_ENCODER_CH1_P02_8, TIM4_ENCODER_CH2_P00_9);

}

void encoderRead()
{
    lEncoderCount = -encoder_get_count(TIM2_ENCODER);
    rEncoderCount = encoder_get_count(TIM3_ENCODER);

    encoder_clear_count(TIM2_ENCODER);
    encoder_clear_count(TIM3_ENCODER);
}

void getSpeed()
{
    int i = 0;
    float sum_r = 0.0;
    float sum_l = 0.0;
    // int32 sum_r_num = 0;
    // int32 sum_l_num = 0;
    static float speed_r[MOVINGAVERAGE_NUM];
    static float speed_l[MOVINGAVERAGE_NUM];

    encoderRead();
    for (i = MOVINGAVERAGE_NUM - 1; i >= 1; i--)
    {
        speed_r[i] = speed_r[i - 1];
        speed_l[i] = speed_l[i - 1];
        // speed_r_num[i] = speed_r_num[i - 1];
        // speed_l_num[i] = speed_l_num[i - 1];
    }
    speed_r[0] = (float)(rEncoderCount) / (float)(RIGHT_1M_ENCODER * TIME_PER);
    speed_l[0] = (float)(lEncoderCount) / (float)(LEFT_1M_ENCODER * TIME_PER);
    // speed_r_num[0] = rEncoderCount;
    // speed_l_num[0] = lEncoderCount;
    for (i = 0; i < MOVINGAVERAGE_NUM; i++)
    {
        sum_r += speed_r[i];
        sum_l += speed_l[i];
        // sum_r_num += speed_r_num[i];
        // sum_l_num += speed_l_num[i];
    }
    // rSpeed_NUMBER = sum_r_num / MOVINGAVERAGE_NUM;
    // lSpeed_NUMBER = sum_l_num / MOVINGAVERAGE_NUM;
    rSpeed = sum_r / MOVINGAVERAGE_NUM;
    lSpeed = sum_l / MOVINGAVERAGE_NUM;
    distance += (rSpeed + lSpeed) * TIME_PER / 2.0;
}