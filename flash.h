#ifndef _FLASH_H_
#define _FLASH_H_

#include "zf_common_headfile.h"

extern flash_data_union flash_union_buffer[];


void readParameterFromFlash(void);
void saveParameterToFlash(void);

#endif