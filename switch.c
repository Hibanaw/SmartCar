#include "switch.h"

uint8 switch1 = 0;
uint8 switch2 = 0;

void switchInit(){
    gpio_init(SWITCH1, GPI, GPIO_HIGH, GPI_FLOATING_IN);
    gpio_init(SWITCH2, GPI, GPIO_HIGH, GPI_FLOATING_IN);
}

void switchScan(){
    switch1 = gpio_get_level(SWITCH1);
    switch2 = gpio_get_level(SWITCH2);
}