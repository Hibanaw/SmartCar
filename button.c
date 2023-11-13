#include "button.h"

#include "zf_common_headfile.h"
#include "param.h"
#include "definition.h"

void buttonInit()
{
  gpio_init(PIN_BUTTON_0, GPI, GPIO_HIGH, GPI_PULL_UP);
  gpio_init(PIN_BUTTON_1, GPI, GPIO_HIGH, GPI_PULL_UP);
  gpio_init(PIN_BUTTON_2, GPI, GPIO_HIGH, GPI_PULL_UP);
  gpio_init(PIN_BUTTON_3, GPI, GPIO_HIGH, GPI_PULL_UP);
}

void buttonScan()
{
  static uint8 key;
  key <<= 4;

  if (!gpio_get_level(PIN_BUTTON_0))
  {
    key += BUTTON_1;
  }
  else if (!gpio_get_level(PIN_BUTTON_1))
  {
    key += BUTTON_2;
  }
  else if (!gpio_get_level(PIN_BUTTON_2))
  {
    key += BUTTON_3;
  }
  else if (!gpio_get_level(PIN_BUTTON_3))
  {
    key += BUTTON_4;
  }
  // 低四位有值的话就不输出，低四位无值则输出相应键值
  g_button = (key & 0x0f) ? BUTTON_NONE : (key >> 4);
}