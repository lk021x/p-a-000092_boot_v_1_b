/********************************************************************************
 * Copyright (C), 1997-2011, SUNGROW POWER SUPPLY CO., LTD. 
 * File name      :rtc.h
 * Author         :Linfei
 * Date           :2012.07.13
 * Description    :DS1340C驱动头文件，模拟I2C方式，移植时需修改该文件中的引脚定义
 *                 部分以及置高置低宏(STM32F10xx)，目前该驱动头文件适用STM32F4xx
 * Others         :None
 *-------------------------------------------------------------------------------
 * 2012.07.11 : 1.0.0 : Linfei
 * Modification   : 初始代码编写。
 ********************************************************************************/
#ifndef _RTC_H_
#define _RTC_H_

#include "stm32f4xx.h"
#include "fcntl.h"

// RTC使用的I2C引脚定义
#define RTC_SDA_PIN      GPIO_Pin_3
#define RTC_SDA_GROUP    GPIOD

#define RTC_SCL_PIN      GPIO_Pin_2
#define RTC_SCL_GROUP    GPIOD


// 总线置高置低宏
#define SCL_H    (RTC_SCL_GROUP->BSRRL = RTC_SCL_PIN)
#define SCL_L    (RTC_SCL_GROUP->BSRRH = RTC_SCL_PIN)  
#define SDA_H    (RTC_SDA_GROUP->BSRRL = RTC_SDA_PIN)
#define SDA_L    (RTC_SDA_GROUP->BSRRH = RTC_SDA_PIN)

// 总线值读取宏
#define SCL_read    (GPIO_ReadInputDataBit(RTC_SCL_GROUP, RTC_SCL_PIN) == Bit_SET)
#define SDA_read    (GPIO_ReadInputDataBit(RTC_SDA_GROUP, RTC_SDA_PIN) == Bit_SET)

// 参考年月日，如果读出的RTC时间小于该日期则报错
#define REF_YEAR    0x12
#define REF_MONTH   0x07
#define REF_DATE    0x13


// 涓流充电器控制命令，通过rtc_ioctl执行，禁止更改宏值!!
#define RTC_CHARGER_ON_250      0xA5    // 充电使能，无二极管，250欧姆电阻
#define RTC_CHARGER_ON_2K       0xA6    // 充电使能，无二极管，2K欧姆电阻
#define RTC_CHARGER_ON_4K       0xA7    // 充电使能，无二极管，4K欧姆电阻
#define RTC_CHARGER_ON_250_D    0xA9    // 充电使能，无二极管，250欧姆电阻
#define RTC_CHARGER_ON_2K_D     0xAA    // 充电使能，无二极管，2K欧姆电阻
#define RTC_CHARGER_ON_4K_D     0xAB    // 充电使能，无二极管，4K欧姆电阻
#define RTC_CHARGER_OFF         0x00    // 关闭充电



//函数声明
int32_t rtc_open(int32_t flag, int32_t mode);
int32_t rtc_read(char *buf, uint32_t count);
int32_t rtc_write(const char *buf, uint32_t count);
int32_t rtc_ioctl(uint32_t op, void *arg);
int32_t rtc_release(void);
#endif
