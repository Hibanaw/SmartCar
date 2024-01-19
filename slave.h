#ifndef _SLAVE_H_
#define _SLAVE_H_

#include "zf_common_typedef.h"
#include "button.h"

enum SLAVE_BUTTON_ID{
    SLAVEBUTTON_UP = BUTTON_2,
    SLAVEBUTTON_DOWN = BUTTON_1,
    SLAVEBUTTON_LEFT = BUTTON_3,
    SLAVEBUTTON_RIGHT = BUTTON_4,
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
    struct SlaveOption option[15];
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
