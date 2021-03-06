#ifndef SYSTICK_H__
#define SYSTICK_H__

#include "stm32f4xx.h"
#include "fcntl.h"


/*地址: 0x00000001 长度: 4 作用: 读取当前系统计时点*/
/*操作: open close read ioctl 其它操作会引发一个错误*/
/*特性: 默认系统时钟为1ms中断一次，如果需要修改其配置则可以通过ioctl操作*/
#define SYSTICK_IOCTL_REDEF 1U /*重新初始化SysTick设备*/



#ifndef  DSUCCESS
 #define  DSUCCESS                      0           /* 操作成功 */
#endif

#ifndef  OPFAULT
 #define  OPFAULT                        (-1)        /* 操作出错 */
#endif

#ifndef DEVBUSY
 #define DEVBUSY                          (-2)        /* 忙 */
#endif



typedef struct {
    uint16_t freq;/*计数点*/
    uint32_t mode;/*模式*/
}TSysTick_ioctl;


#define SYSTICK_TIMER      \
    do                     \
    {                      \
        dSysTick_timer();  \
    }while(0)           

void dSysTick_timer(void);

int32_t dSysTick_open(int32_t flags, int32_t mode);
int32_t dSysTick_release(void);
int32_t dSysTick_read (uint8_t buf[], uint32_t count);
int32_t dSysTick_ioctl(uint32_t cmd,void* arg);

#endif
