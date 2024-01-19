#include "reset.h"

#include "param.h"
#include "image.h"

void checkReset(){
    if(reset){
        routeState = STOP;
        steerError = 0;
        steerTarget = 0;
        system_delay_ms(2000);
        if(doOutPark)
            routeState = PARK_OUT;
        stateTime = system_getval_ms();
        reset = 0;
    }
}