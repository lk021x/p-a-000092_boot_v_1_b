/********************************************************************************
 * Copyright (C), 1997-2011, SUNGROW POWER SUPPLY CO., LTD. 
 * File name      :extwdg.h
 * Author         :Linfei
 * Date           :2012.08.30
 * Description    :外部看门狗TPS3823驱动，喂狗脉冲2us，超时复位时间固定为1.6s
 * Others         :None
 *-------------------------------------------------------------------------------
 * 2012.08.30 : 1.0.0 : Linfei
 * Modification   : 初始代码编写。
 ********************************************************************************/
#ifndef IWDG_H__
#define IWDG_H__
#include "stm32f4xx.h"
#include "fcntl.h"

/* 外部看门狗 */
#define EWDG_Pin          (GPIO_Pin_5)
#define EWDG_GROUP        (GPIOA)


#ifndef  DSUCCESS
 #define  DSUCCESS                      0           /* 操作成功 */
#endif

#ifndef  OPFAULT
 #define  OPFAULT                       (-1)        /* 操作出错 */
#endif

#ifndef DEVBUSY
 #define DEVBUSY                        (-2)        /* 忙 */
#endif

#define FEED_EXTWDG         0x01U

int32_t extwdg_open(int32_t flag, int32_t mode);
int32_t extwdg_ioctl(uint32_t op, void *arg);
int32_t extwdg_close(void);


#endif
