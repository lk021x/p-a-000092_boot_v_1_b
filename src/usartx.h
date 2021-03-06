/****************************************************************************** 
 * Copyright (C), 1997-2011, SUNGROW POWER SUPPLY CO., LTD. 
 * File name      :usartx.h 
 * Author         :Xu Shun'an
 * Date           :2011.05.27
 * Description    :STM32F10x系列处理器的串口模块函数的头文件，定义了串口个数、
 *                 默认波特率、PCB板485接口的控制引脚所对应的端口及端口号、进
 *                 行串口配置的类型、串口基本设置以及串口驱动函数
 * Others         :None
 *******************************************************************************
 *------------------------------------------------------------------------------
 * 2011-06-01 : 1.0.1 : xusa
 * Modification   :  整理代码格式
 *------------------------------------------------------------------------------
 * 2010-08-03 : 1.0.0 : liulei
 * Modification   : 初始代码编写。
 ********************************************************************************/
#ifndef USARTX_H_
#define USARTX_H_


/*lint -e40 *//*由于STM32库stm32f10x.h中的USARTx_IRQn未定义数据类型*/
/*lint -e506 *//*该警告目前还没有找到解决办法*/

#include "string.h"
#include "stm32f4xx_usart.h"
#include "fcntl.h"

typedef struct
{
    /*串口基本设置*/
    uint32_t baudrate; /*波特率(1200~256000)*/
    uint16_t wordlength; /*位长*///wujing 2012.12.11 for xu
    uint16_t stopbits; /*停止位*/
    uint16_t parity; /*效验方式*/
    uint8_t flowcontrol; /*流控*/
    uint8_t mode; /*模式*/

    /*串口缓冲设置*/
    uint8_t *sbuf; /*发送缓冲*/
    uint16_t sbuflen; /*缓冲长度*/
    uint8_t *rbuf; /*接收缓冲*/
    uint16_t rbuflen; /*缓冲长度*/
} TUsartX_ioctl;


/*处理器中usart硬件的个数*/
#define USART_NUM    5U

/*打开串口后默认的波特率*/
#define DEFAULT_BAUD    9600U

/*各串口的485控制引脚配置,用户需在这里将PCB板对应的485接口控制引脚写入*/
/*比如A0，B0的使用的是USART1，控制引脚使用的端口是A口第8个PA8，*/
/*则在PORT_485_1、PIN_485_1处写上GPIOA、GPIO_Pin_8*/
#define PORT_485_1          (GPIOB)
#define PIN_485_1           (GPIO_Pin_4)

#define PORT_485_2          (GPIOG)
#define PIN_485_2           (GPIO_Pin_10)

#define PORT_485_3          (GPIOD)
#define PIN_485_3           (GPIO_Pin_11)

#define PORT_485_4          (NULL)
#define PIN_485_4           (NULL)

#define PORT_485_5          (NULL)
#define PIN_485_5           (NULL)

/*对串口进行的操作种类*/
#define USART_SET_PORT    1U       /*重新初始化串口*/
#define USART_SET_BUF     2U       /*设置缓冲区*/
#define USART_SET_485E    3U       /*开启485支持*/
#define USART_SET_485D    4U       /*禁止485支持*/
#define USART_SET_SYNC    5U       /*设置为同步模式*/
#define USART_SET_ASYN    6U      /*设置为异步模式*/


/*驱动用定时器(1ms)*/
/*在这里加入驱动需要使用的定时器函数*/
#define UART_TIMER      \
    do                  \
    {                   \
        usartx_timer(); \
    }while(0)           


/*串口通信总线空闲判断函数*/
void usartx_timer(void);


/*usart1驱动*/
int32_t usart1_open(int32_t flags, int32_t mode);
int32_t usart1_close(void);
int32_t usart1_write(const uint8_t buf[], uint32_t count);
int32_t usart1_read(uint8_t buf[], uint32_t count);
int32_t usart1_ioctl(uint32_t cmd, void *arg);

/*usart2驱动*/
int32_t usart2_open(int32_t flags, int32_t mode);
int32_t usart2_close(void);
int32_t usart2_write(const uint8_t buf[], uint32_t count);
int32_t usart2_read(uint8_t buf[], uint32_t count);
int32_t usart2_ioctl(uint32_t cmd, void *arg);

/*usart3驱动*/
int32_t usart3_open(int32_t flags, int32_t mode);
int32_t usart3_close(void);
int32_t usart3_write(const uint8_t buf[], uint32_t count);
int32_t usart3_read(uint8_t buf[], uint32_t count);
int32_t usart3_ioctl(uint32_t cmd, void *arg);

/*usart4驱动*/
int32_t usart4_open(int32_t flags, int32_t mode);
int32_t usart4_close(void);
int32_t usart4_write(const uint8_t buf[], uint32_t count);
int32_t usart4_read(uint8_t buf[], uint32_t count);
int32_t usart4_ioctl(uint32_t cmd, void *arg);

/*usart5驱动*/
int32_t usart5_open(int32_t flags, int32_t mode);
int32_t usart5_close(void);
int32_t usart5_write(const uint8_t buf[], uint32_t count);
int32_t usart5_read(uint8_t buf[], uint32_t count);
int32_t usart5_ioctl(uint32_t cmd, void *arg);

#if SYS_OS_SUPPORT
int32_t usart1_poll(void *ev);
int32_t usart2_poll(void *ev);
int32_t usart3_poll(void *ev);
int32_t usart4_poll(void *ev);
int32_t usart5_poll(void *ev);
#endif


#ifndef  DSUCCESS
 #define  DSUCCESS                      0           /* 操作成功 */
#endif

#ifndef  OPFAULT
 #define  OPFAULT                        (-1)        /* 操作出错 */
#endif

#ifndef DEVBUSY
 #define DEVBUSY                          (-2)        /* 忙 */
#endif

#endif



