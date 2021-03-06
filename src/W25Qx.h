/****************************************************************************** 
 * Copyright (C), 1997-2011, SUNGROW POWER SUPPLY CO., LTD. 
 * File name      :W25Qx.h 
 * Author         :Xu Shun'an
 * Date           :2011.06.04
 * Description    :STM32F10x系列处理器的型号为W25Q型FLASH模块函数的头文件，定义
 *                 FLASH型号、管脚配置、FLASH擦除命令、访问FLASH的参数结构以及
 *                 功能函数
 * Others         :None
 *-------------------------------------------------------------------------------
 * 2011-06-08 : 1.0.1 : xusa
 * Modification   :  整理代码格式，有返回值的函数进行处理，主要是根据返回值判断下
 *                   一步进行何种操作
 *-------------------------------------------------------------------------------
 * 2010-08-03 : 1.0.0 : liulei
 * Modification   : 初始代码编写。
 ********************************************************************************/
#ifndef W25QX_H
#define W25QX_H

/*lint -e534 */ /*该警告由于函数返回值确实不用判断，情况是可控的 */

#include "stm32f4xx.h"
#include "fcntl.h"

/* 芯片型号选择，W25Q型FLASH共有四款，目前用到的是W25Q64BV，有8MB存储空间 */
#define W25Q80BV            0
#define W25Q16BV            0
#define W25Q32BV            0
#define W25Q64QV            1



/* SPI管脚配置(此处使用未被重映射的SPI1)，FLASH芯片作为从设备，STM32为主设备 */
#define W25QX_SPI           (SPI3)
/* 开启时钟 */
#define W25QX_SPI_RCC_OPEN           (RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3,ENABLE))


/* W25QX芯片片选管脚 */
#define W25Q_PORT_CS         (GPIOA)
#define W25Q_PIN_CS          (GPIO_Pin_15)

/* ioctl命令，分别代表整个芯片擦除命令、页擦除命令、块擦除命令、段擦除命令 */
#define W25QX_CMD_BE        0x01U
#define W25QX_CMD_PE        0x02U
#define W25QX_CMD_SSE       0x03U
#define W25QX_CMD_SE        0x07U
#define W25QX_CMD_BE32KB    0x08U
#define W25QX_CMD_BE64KB    0x09U


/* FLASH大小 */
#if (W25Q80BV == 1)
#define W25Q_FLASH_SIZE 0x100000     /* 1MB */
#endif

#if (W25Q16BV == 1)
#define W25Q_FLASH_SIZE 0x200000     /* 2MB */
#endif

#if (W25Q32BV == 1)
#define W25Q_FLASH_SIZE 0x400000     /* 4MB */
#endif

#if (W25Q64QV == 1)
#define W25Q_FLASH_SIZE 0x800000     /* 8MB */
#endif

/* ioctl参数结构体，分别代表数据和地址 */
typedef struct{
    uint8_t* dat;
    int64_t addr;
}W25QX_IOCTL_ARG;


#ifndef  DSUCCESS
 #define  DSUCCESS                      0           /* 操作成功 */
#endif

#ifndef  OPFAULT
 #define  OPFAULT                       (-1)        /* 操作出错 */
#endif

#ifndef DEVBUSY
 #define DEVBUSY                        (-2)        /* 忙 */
#endif

int32_t w25qx_open(int32_t flag, int32_t mode);
int32_t w25qx_close(void);
int32_t w25qx_read(uint8_t buf[], uint32_t count);
int32_t w25qx_write(const uint8_t buf[], uint32_t count);
int32_t w25qx_lseek(int64_t offset, int32_t where);
int32_t w25qx_ioctl(uint32_t op, void* arg);

#endif

