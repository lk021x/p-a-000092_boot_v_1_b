/****************************************************************************** 
* Copyright (C), 1997-2010, SUNGROW POWER SUPPLY CO., LTD. 
* File name      :device.c
* Author         :llemmx
* Date           :2011-05-03
* Description    :实现设备文件操作接口，与硬件初始化接口。
* Others         :无
*******************************************************************************
* History:        初稿
*------------------------------------------------------------------------------
* 2011-05-03 : 1.0.0 : llemmx
* Modification   :
*------------------------------------------------------------------------------
******************************************************************************/
#include "stm32f4xx_rcc.h"
#include "device.h"


//驱动接口
typedef struct {
    int32_t (*lseek)   (int64_t, int32_t);
    int32_t (*read)    (uint8_t *, uint32_t);
    int32_t (*write)   (const uint8_t *, uint32_t);
    int32_t (*ioctl)   (uint32_t,void*);
    int32_t (*open)    (int32_t,int32_t);
    int32_t (*release) (void);
    char *drvname;
}file_operations;


//全局设备结构体(静态)
file_operations const g_drv[]={
#if DRV_SYSTICK
    {.open=dSysTick_open,.release=dSysTick_release,.read=dSysTick_read,
     .write=NULL,.lseek=NULL,.ioctl=dSysTick_ioctl,.drvname="SYSTICK0"},      //SysTick
#endif
     
#if DRV_USARTX
    {.open=usart1_open,.release=usart1_close,.read=usart1_read,
     .write=usart1_write,.lseek=NULL,.ioctl=usart1_ioctl,.drvname="USART1"},//usart1
    {.open=usart2_open,.release=usart2_close,.read=usart2_read,
     .write=usart2_write,.lseek=NULL,.ioctl=usart2_ioctl,.drvname="USART2"},//usart2
    {.open=usart3_open,.release=usart3_close,.read=usart3_read,
     .write=usart3_write,.lseek=NULL,.ioctl=usart3_ioctl,.drvname="USART3"},//usart3
    {.open=usart4_open,.release=usart4_close,.read=usart4_read,
     .write=usart4_write,.lseek=NULL,.ioctl=usart4_ioctl,.drvname="USART4"},//usart4
    {.open=usart5_open,.release=usart5_close,.read=usart5_read,
     .write=usart5_write,.lseek=NULL,.ioctl=usart5_ioctl,.drvname="USART5"},//usart5
#endif
     
#if DRV_CANX
    {.open=can1_open,.release=can1_release,.read=can1_read,
     .write=can1_write,.lseek=NULL,.ioctl=can1_ioctl,.drvname="CAN1"},     //CAN1          //123456 两路CAN如何定义，是不是直接在后面加载，改变号码就行
    {.open=can2_open,.release=can2_release,.read=can2_read,
     .write=can2_write,.lseek=NULL,.ioctl=can2_ioctl,.drvname="CAN2"},     //CAN2    
#endif 
     
#if DRV_EEPROM
    {.open=EEPROM_open,.release=EEPROM_close,.read=EEPROM_read,
     .write=EEPROM_write,.lseek=EEPROM_lseek,.ioctl=EEPROM_ioctl,.drvname="FM31XX0"},//fm31xx
#endif
     
#if DRV_W25Qx
    {.open=w25qx_open,.release=w25qx_close,.read=w25qx_read,
     .write=w25qx_write,.lseek=w25qx_lseek,.ioctl=w25qx_ioctl,.drvname="W25QX0"},//W25q16bv
#endif
     
#if DRV_ADC
    {.open=adc3_open,.release=adc3_release,.read=adc3_read,
     .write=NULL,.lseek=NULL,.ioctl=adc3_ioctl,.drvname="ADC3"},//ADC3
#endif
     
#if DRV_RTC
    {.open=rtc_open,.release=rtc_release,.read=rtc_read,
     .write=rtc_write,.lseek=NULL,.ioctl=rtc_ioctl,.drvname="RTC0"},//RTC
#endif
     
#if DRV_ADS1247
    {.open=ads1247_open,.release=ads1247_close,.read=ads1247_read,
     .write=NULL,.lseek=NULL,.ioctl=ads1247_ioctl,.drvname="ADS1247"},//RTC
#endif

#if DRV_EXTWDG
    {.open=extwdg_open,.release=extwdg_close,.read=NULL,
     .write=NULL,.lseek=NULL,.ioctl=extwdg_ioctl,.drvname="EXTWDG0"},//RTC
#endif

};

static uint8_t g_DrvMaxNum;
ErrorStatus HSEStartUpStatus;






extern void init_interface(void);


/******************************************************************************
* Function       :RCC_Configuration
* Author         :llemmx
* Date           :2011-05-03
* Description    :配置系统时钟
* Calls          :无
* Table Accessed :无
* Table Updated  :无
* Input          :无
* Output         :无
* Return         :无
*******************************************************************************
* History:        2011-05-03 
*------------------------------------------------------------------------------
* 2011-05-03 : 1.0.0 : llemmx
* Modification   : 创建
*------------------------------------------------------------------------------
******************************************************************************/
static void RCC_Configuration(void)	
{
  /* FPU settings ------------------------------------------------------------*/
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */
#endif


	/* RCC system reset(for debug purpose) */
    RCC_DeInit();

#ifdef DATA_IN_ExtSRAM
    SystemInit_ExtMemCtl(); 
#endif /* DATA_IN_ExtSRAM */

    /* Enable HSE */
    RCC_HSEConfig(RCC_HSE_ON);

    /* Wait till HSE is ready */
    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    if(HSEStartUpStatus == SUCCESS)
    {
        /* Configure Flash prefetch, Instruction cache, Data cache and wait state */
        //FLASH_PrefetchBufferCmd(ENABLE);
        //FLASH_InstructionCacheCmd(ENABLE);
        //FLASH_DataCacheCmd(ENABLE);
        FLASH_SetLatency(FLASH_Latency_5); // VCC = 3.3V, HCLK = 168MHz

        /* HCLK = SYSCLK */
        RCC_HCLKConfig(RCC_SYSCLK_Div1); 

        /* PCLK2 = HCLK / 2 */
        RCC_PCLK2Config(RCC_HCLK_Div2); 

        /* PCLK1 = HCLK / 4 */
        RCC_PCLK1Config(RCC_HCLK_Div4);

        /* ADCCLK = PCLK2/4 */
        //RCC_ADCCLKConfig(RCC_PCLK2_Div4);

        // VCO input Clock: 25MHz / PLLM(25) = 1MHz
        // VCO output Clock: 1MHz * PLLN(336) = 336MHz
        // PLLCLK: 336MHz / PLLP(2) = 168MHz
        // USB SDIO RNG: 336MHz / PLLQ(7) = 48MHz
        RCC_PLLConfig(RCC_PLLSource_HSE, 25, 336, 2, 7);

        /* Enable PLL */ 
        RCC_PLLCmd(ENABLE);

        /* Wait till PLL is ready */
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
        {
        }
        
         

        /* Select PLL as system clock source */
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        /* Wait till PLL is used as system clock source */
        while(RCC_GetSYSCLKSource() != 0x08)
        {
        }
    }

    // GPIO Enable
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | \
                           RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | \
                           RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF | \
                           RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_GPIOH | \
                           RCC_AHB1Periph_GPIOI, ENABLE);
    // Periph Enable
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 | RCC_APB1Periph_USART3 | RCC_APB1Periph_SPI3, ENABLE);
    
    /* Enable SYSCFG clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    
    /* Enable ETHERNET clock  */
//     RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC | RCC_AHB1Periph_ETH_MAC_Tx |
//                           RCC_AHB1Periph_ETH_MAC_Rx, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
	//usb
	//RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
}

/******************************************************************************
* Function       :NVIC_Configuration
* Author         :llemmx
* Date           :2011-05-03
* Description    :Configures the nested vectored interrupt controller.
* Calls          :无
* Table Accessed :无
* Table Updated  :无
* Input          :无
* Output         :无
* Return         :无
*******************************************************************************
* History:        2011-05-03 
*------------------------------------------------------------------------------
* 2011-05-03 : 1.0.0 : llemmx
* Modification   : 创建
*------------------------------------------------------------------------------
******************************************************************************/
void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* Set the Vector Table base location at 0x08000000 */ 
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x08000000);
	
	/* Configure one bit for preemption priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	// Enable the USART1 Interrupt 
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

    // Enable the USART2 Interrupt 
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;	   
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

	// Enable the USART3 Interrupt 
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);	  

	/* Enable the WDG Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = WWDG_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
/*
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
*/
    /* Enable and set EXTI Line14 Interrupt to the highest priority */
//     NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
//     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
//     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
//     NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//     NVIC_Init(&NVIC_InitStructure);

//     /* Enable the Ethernet global Interrupt */
//     NVIC_InitStructure.NVIC_IRQChannel = ETH_IRQn;
//     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
//     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//     NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//     NVIC_Init(&NVIC_InitStructure);
}


// 外部掉电检测中断初始化
void EXTI_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line14; //外部中断线
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断模式
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;//中断触发方式：上升沿
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;//打开中断
    EXTI_Init(&EXTI_InitStructure);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, GPIO_PinSource14);  // 掉电检测外部中断
}



/*******************************************************************************
* Function Name  : OTG_FS_IRQHandler
* Description    : This function handles USB-On-The-Go FS global interrupt request.
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/*void OTG_FS_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR cpu_sr;
#endif

    OS_ENTER_CRITICAL();    // Tell uC/OS-II that we are starting an ISR
    OSIntNesting++;
    OS_EXIT_CRITICAL();
    
    STM32_PCD_OTG_ISR_Handler();
    
    OSIntExit();         // Tell uC/OS-II that we are leaving the ISR
}*/

/*******************************************************************************
* Function Name  : MAL_Config
* Description    : MAL_layer configuration
* Input          : None.
* Return         : None.
*******************************************************************************/
/*void MAL_Config(void)
{
    MAL_Init(0);
}*/

/******************************************************************************
* Function       :iwdg_init
* Author         :llemmx
* Date           :2011-05-28
* Description    :喂狗初始化
* Calls          :无
* Table Accessed :无
* Table Updated  :无
* Input          :无
* Output         :无
* Return         :初始化成功返回TRUE，初始化失败返回FALSE
*******************************************************************************
* History:        2011-05-03 
*------------------------------------------------------------------------------
* 2011-05-03 : 1.0.0 : llemmx
* Modification   : 创建
*------------------------------------------------------------------------------
******************************************************************************/
void iwdg_init(void)
{
    /* IWDG timeout equal to 600 ms (the timeout may varies due to LSI frequency
        dispersion) */
    /* Enable write access to IWDG_PR and IWDG_RLR registers */
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

    /* IWDG counter clock: 40KHz(LSI) / 64 = 625 Hz */
    IWDG_SetPrescaler(IWDG_Prescaler_64);

    /* Set counter reload value to 375 */
    IWDG_SetReload(0xFFF);

    /* Reload IWDG counter */
    IWDG_ReloadCounter();

    /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
    IWDG_Enable();
}
/******************************************************************************
* Function       :dev_init
* Author         :llemmx
* Date           :2011-05-03
* Description    :设备初始化
* Calls          :无
* Table Accessed :无
* Table Updated  :无
* Input          :无
* Output         :无
* Return         :初始化成功返回TRUE，初始化失败返回FALSE
*******************************************************************************
* History:        2011-05-03 
*------------------------------------------------------------------------------
* 2011-05-03 : 1.0.0 : llemmx
* Modification   : 创建
*------------------------------------------------------------------------------
******************************************************************************/
BOOL dev_init(void)
{
    //初始化系统时钟
    RCC_Configuration();
    
    if(RCC_GetFlagStatus(RCC_FLAG_WWDGRST) != RESET)
    {  
      // Clear reset flags 
      RCC_ClearFlag();
    }

    //中断向量
    NVIC_Configuration();
    //初始化计时器(1ms)   
    
    g_DrvMaxNum=(sizeof(g_drv)/sizeof(file_operations));
    //MAL_Config();//存储设备初始化
    
    //init io interface
    init_interface();
    
    //open iwdg
    //iwdg_init();
    //EXTI_Configuration();  // xusa 2011.9.14
    
    return TRUE;
}


/******************************************************************************
* Function       :open
* Author         :llemmx
* Date           :2011-05-03
* Description    :打开驱动文件
* Calls          :无
* Table Accessed :无
* Table Updated  :无
* Input          :pathname - 文件名称
                  flags - 打开文件标志
                  mode - 模式
* Output         :无
* Return         :打开成功返回文件句柄，失败返回EFAULT
*******************************************************************************
* History:        2011-05-03 
*------------------------------------------------------------------------------
* 2011-05-03 : 1.0.0 : llemmx
* Modification   : 创建
*------------------------------------------------------------------------------
******************************************************************************/
int32_t open(const char* pathname, int32_t flags, int32_t mode)
{
    uint8_t i;
    int32_t ret;
    for (i=0;i<g_DrvMaxNum;++i)
    {
        if (strcmp(pathname,g_drv[i].drvname)==0)
        {
            ret=g_drv[i].open(flags,mode);
            if (DSUCCESS!=ret)
            {
                return ret;
            }
            return i+1;
        }
    }
    return EFAULT;
}

/******************************************************************************
* Function       :close
* Author         :llemmx
* Date           :2011-05-06
* Description    :关闭文件
* Calls          :无
* Table Accessed :无
* Table Updated  :无
* Input          :fd - 文件句柄
* Output         :无
* Return         :成功返回DSUCCESS，失败返回EFAULT
*******************************************************************************
* History:        2011-05-03 
*------------------------------------------------------------------------------
* 2011-05-03 : 1.0.0 : llemmx
* Modification   : 创建
*------------------------------------------------------------------------------
******************************************************************************/
int32_t close(int32_t fd)
{
    int32_t ret;
    if (fd>g_DrvMaxNum || fd == 0)
    {
        return EFAULT;
    }
    if (NULL==g_drv[fd-1].release)
    {
        return EFAULT;
    }

    ret=g_drv[fd-1].release();
    return ret;
}

/******************************************************************************
* Function       :read
* Author         :llemmx
* Date           :2011-05-06
* Description    :关闭文件
* Calls          :无
* Table Accessed :无
* Table Updated  :无
* Input          :fd - 文件句柄
                  count - 缓冲区长度
* Output         :buf - 缓冲区指针
* Return         :成功返回实际读取数据长度，失败返回EFAULT
*******************************************************************************
* History:        2011-05-03 
*------------------------------------------------------------------------------
* 2011-05-03 : 1.0.0 : llemmx
* Modification   : 创建
*------------------------------------------------------------------------------
******************************************************************************/
int32_t read(int32_t fd, void* buf, uint32_t count)
{
    int32_t ret;
    //系统计数驱动
    if (fd>g_DrvMaxNum || fd == 0)
    {
        return EFAULT;
    }
    if (NULL==g_drv[fd-1].read)
    {
        return EFAULT;
    }
    
    ret=g_drv[fd-1].read(buf,count);
    return ret;
}

/******************************************************************************
* Function       :write
* Author         :llemmx
* Date           :2011-05-06
* Description    :写数据
* Calls          :无
* Table Accessed :无
* Table Updated  :无
* Input          :fd - 文件句柄
                  count - 缓冲区长度
* Output         :buf - 缓冲区指针
* Return         :成功返回实际写入数据长度，失败返回EFAULT
*******************************************************************************
* History:        2011-05-03 
*------------------------------------------------------------------------------
* 2011-05-03 : 1.0.0 : llemmx
* Modification   : 创建
*------------------------------------------------------------------------------
******************************************************************************/
int32_t write(int32_t fd, const void* buf, uint32_t count)
{
    int32_t ret;
    //系统计数驱动
    if (fd>g_DrvMaxNum || fd == 0)
    {
        return EFAULT;
    }
    if (NULL==g_drv[fd-1].write)
    {
        return EFAULT;
    }
    
    ret=g_drv[fd-1].write(buf,count);
    return ret;
}

/******************************************************************************
* Function       :ioctl
* Author         :llemmx
* Date           :2011-05-06
* Description    :驱动的特殊操作
* Calls          :无
* Table Accessed :无
* Table Updated  :无
* Input          :fildes - 文件句柄
                  request - 用户自定义的操作命令
                  arg - 传入的用户参数指针
* Output         :无
* Return         :成功返回DSUCCESS，失败返回EFAULT
*******************************************************************************
* History:        2011-05-03 
*------------------------------------------------------------------------------
* 2011-05-03 : 1.0.0 : llemmx
* Modification   : 创建
*------------------------------------------------------------------------------
******************************************************************************/
int32_t ioctl(int32_t fildes, int32_t request,void* arg)
{
    int32_t ret;
    //系统计数驱动
    if (fildes>g_DrvMaxNum || fildes == 0)
    {
        return EFAULT;
    }
    if (NULL==g_drv[fildes-1].ioctl)
    {
        return EFAULT;
    }
    
    ret=g_drv[fildes-1].ioctl(request,arg);
    return ret;
}

/******************************************************************************
* Function       :lseek
* Author         :llemmx
* Date           :2011-05-06
* Description    :跳转读写指针
* Calls          :无
* Table Accessed :无
* Table Updated  :无
* Input          :fd - 文件句柄
                  offset - 跳转的相对位置
                  whence - 跳转的起始点
* Output         :无
* Return         :成功返回当前指针位置，失败返回EFAULT
*******************************************************************************
* History:        2011-05-03 
*------------------------------------------------------------------------------
* 2011-05-03 : 1.0.0 : llemmx
* Modification   : 创建
*------------------------------------------------------------------------------
******************************************************************************/
int32_t lseek(int32_t fd, long offset, int32_t whence)
{
    int32_t ret;
    //系统计数驱动
    if (fd>g_DrvMaxNum || fd == 0)
    {
        return EFAULT;
    }
    if (NULL==g_drv[fd-1].lseek)
    {
        return EFAULT;
    }
    
    ret=g_drv[fd-1].lseek(offset,whence);

    return ret;
}

