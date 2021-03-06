#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <stdint.h>
#include <string.h>
#include "stm32f4xx.h"

#define NULL  0
#define TRUE  1
#define FALSE 0
#define __IO  volatile                  /*!< defines 'read / write' permissions   */

#define DBPRINTF(format, arg...) { printf("%s:%d::",__FILE__,__LINE__);printf(format, ## arg); }

#endif
