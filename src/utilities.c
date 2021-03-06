#include "utilities.h"

void lmemcpy(uint8_t *buf,uint8_t *src,uint32_t size)
{
    for (;size>0;--size,buf++,src++){
        *buf=*src;
    }
}

void lmemset(uint8_t *buf,uint8_t dat,uint32_t size)
{
    for (int i=0;i<size;++i)
        buf[i]=dat;
}
