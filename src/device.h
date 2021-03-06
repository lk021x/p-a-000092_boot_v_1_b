#ifndef _DEVICE_H_
#define _DEVICE_H_


//驱动定义
#define DRV_SYSTICK   1
#define DRV_USARTX    1
#define DRV_CANX      0
#define DRV_EEPROM    1
#define DRV_W25Qx     1
#define DRV_ADC       0
#define DRV_RTC       0
#define DRV_ADS1247   0
#define DRV_EXTWDG    1


//SysTick----------------------------------------------------------------------
#if DRV_SYSTICK
#include "systick.h"
#endif

//串口驱动---------------------------------------------------------------------
#if DRV_USARTX
#include "usartx.h"
#endif

//can驱动----------------------------------------------------------------------
#if DRV_CANX
#include "canx.h"
#endif 

//EEPROM驱动-------------------------------------------------------------------
#if DRV_EEPROM
#include "EEPROM.h"
#endif

//W25Qx------------------------------------------------------------------------
#if DRV_W25Qx
#include "W25Qx.h"
#endif

//RTC驱动----------------------------------------------------------------------
#if DRV_RTC
#include "rtc.h"
#endif

//DRV_ADC-----------------------------------------------------------------------
#if DRV_ADC
#include "adc.h"
#endif


#if DRV_ADS1247
#include "ads1247.h"
#endif

#if DRV_EXTWDG
#include "extwdg.h"
#endif

#define FM_EEPROM_BASE      0        //EEPROM基址址
#define FM_EEPROM_LEN       0x8000      //EEPROM长度(不同芯片需要修改)
#define FM_EEPROM_END       (FM_EEPROM_BASE+FM_EEPROM_LEN-1)    //EEPROM尾址  

BOOL dev_init(void);               // 设备初始化函数
void Ethernet_Configuration(void); // 网络控制器初始化



// 设备访问接口
int32_t open(const char*, int32_t, int32_t);
int32_t read(int32_t, void*, uint32_t);
int32_t write(int32_t, const void*, uint32_t);
int32_t lseek(int32_t, long, int32_t);
int32_t close(int32_t);
int32_t ioctl(int32_t, int32_t,void *);



#endif

