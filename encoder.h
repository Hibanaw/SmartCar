#ifndef _ENCODER_H_
#define _ENCODER_H_

#define TIME_PER            (0.002)//2MS
#define RIGHT_1M_ENCODER    (11609)
#define LEFT_1M_ENCODER     (11609)
#define MOVINGAVERAGE_NUM   (5)


extern float rSpeed;
extern float lSpeed;
extern float distance;

void encoderInit();
void encoderRead();
void getSpeed();

#endif