#include "flash.h"

void readParameterFromFlash(void)
{
  flash_read_page_to_buffer(126, 3);
  if(flash_union_buffer[255].int32_type != -1){
    memset(flash_union_buffer, 0, sizeof(flash_union_buffer));
    flash_union_buffer[255].int32_type = -1;
  }
}

void saveParameterToFlash(void)
{
  flash_erase_page(126, 3);
  flash_write_page_from_buffer(126, 3);
}