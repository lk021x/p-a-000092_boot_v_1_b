#ifndef __UTILITIES_H_
#define __UTILITIES_H_

#include <stdint.h>

#define DSUCCESS     0
#define EFAULT      -1
#define EBUSY       -16

#define SEEK_SET     0
#define SEEK_CUR     1
#define SEEK_END     2

void lmemcpy(uint8_t *buf,uint8_t *src,uint32_t size);
void lmemset(uint8_t *buf,uint8_t dat,uint32_t size);

#endif
