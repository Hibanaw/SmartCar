#ifndef _BUTTON_H_
#define _BUTTON_H_

enum BUTTON_ID
{
  BUTTON_NONE = 0,
  BUTTON_1 = 1,
  BUTTON_2 = 2,
  BUTTON_3 = 3,
  BUTTON_4 = 4
};


void buttonInit();
void buttonScan();

#endif