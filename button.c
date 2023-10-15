#include "button.h"


void buttonInit()
{
  gpio_init(D0, GPI, GPIO_HIGH, GPI_PULL_UP);
  gpio_init(D1, GPI, GPIO_HIGH, GPI_PULL_UP);
  gpio_init(D2, GPI, GPIO_HIGH, GPI_PULL_UP);
  gpio_init(D3, GPI, GPIO_HIGH, GPI_PULL_UP);
}

void buttonScan()
{
  static uint8 key;
  key <<= 4;

  if (!gpio_get_level(D0))
  {
    key += BUTTON_1;
  }
  else if (!gpio_get_level(D1))
  {
    key += BUTTON_2;
  }
  else if (!gpio_get_level(D2))
  {
    key += BUTTON_3;
  }
  else if (!gpio_get_level(D3))
  {
    key += BUTTON_4;
  }
  // 低四位有值的话就不输出,低四位无值则输出相应键值
  g_button = (key & 0x0f) ? BUTTON_NONE : (key >> 4);
}