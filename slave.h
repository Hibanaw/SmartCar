#ifndef _SLAVE_H_
#define _SLAVE_H_

#include "zf_common_headfile.h"

#include <math.h>

#include "param.h"
#include "flash.h"
#include "button.h"
#include "pwm.h"
#include "switch.h"
#include "image.h"

enum SLAVE_BUTTON_ID{
    SLAVEBUTTONUP = BUTTON_3,
    SLAVEBUTTONDOWN = BUTTON_4,
    SLAVEBUTTONOK = BUTTON_2,
    SLAVEBUTTONBACK = BUTTON_1
};

struct SlaveOption
{
    char* name;
    enum OptionType{
        VALUE,
        SWITCH
    } type;
    int32 lRange;
    int32 rRange;
    int32 data;
    int32 tempData;
};


struct SlavePage
{
    char* title;
    struct SlaveOption option[10];
    uint8 optionCount;
};


void slaveInit();
void slaveSave();
void slaveWelcome();
void slaveEvent();
void slaveDraw();
void slaveShowStatus();
void slaveShowImage();
#endif