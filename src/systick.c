#include "systick.h"
#include "fcntl.h"
/*Sys Tick驱动*/
/*该驱动为系统的核心驱动*/
static uint8_t    lsystick_opened=0U;
volatile uint32_t lsystick_num=0U;
static uint32_t SysTick_Config(uint32_t ticks);

/******************************************************************************
* Function       :dSysTick_timer
* Author         :Linfei
* Date           :2012.09.19
* Description    :关闭系统定时器设备
* Calls          :None
* Input          :None
* Output         :None
* Return         :None
*******************************************************************************
* History:
*------------------------------------------------------------------------------
* 2012.09.19 : 1.0.0 : Linfei
* Modification   : 创建
*------------------------------------------------------------------------------
******************************************************************************/
void dSysTick_timer(void)
{
    ++lsystick_num;
}

/******************************************************************************
* Function       :dSysTick_open
* Author         :llemmx
* Date           :2011-05-03
* Description    :打开系统定时器设备
* Calls          :无
* Table Accessed :无
* Table Updated  :无
* Input          :flags - 打开标志
                  mode - 模式
* Output         :无
* Return         :打开成功返回DSUCCESS，失败返回OPFAULT
*******************************************************************************
* History:        2011-05-03 
*------------------------------------------------------------------------------
* 2011-05-03 : 1.0.0 : llemmx
* Modification   : 创建
*------------------------------------------------------------------------------
******************************************************************************/
int32_t dSysTick_open(int32_t flags, int32_t mode)
{
    int32_t ret = OPFAULT;
	/*该设备仅能被打开一次*/
    if (lsystick_opened != 1U)
    {
    	++lsystick_opened;
    	/*默认配置*/
    	(void)SysTick_Config(21000U);
    	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);/*系统时钟8分频*/
    	ret = DSUCCESS;
    }
    return ret;
}

/******************************************************************************
* Function       :dSysTick_release
* Author         :llemmx
* Date           :2011-05-03
* Description    :关闭系统定时器设备
* Calls          :无
* Table Accessed :无
* Table Updated  :无
* Input          :无
* Output         :无
* Return         :打开成功返回DSUCCESS。
*******************************************************************************
* History:        2011-05-03 
*------------------------------------------------------------------------------
* 2011-05-03 : 1.0.0 : llemmx
* Modification   : 创建
*------------------------------------------------------------------------------
******************************************************************************/
int32_t dSysTick_release(void)
{
	int32_t ret;
    if (lsystick_opened == 0U)
    {
        ret = DSUCCESS;
    }
    else
    {
		lsystick_opened--;
		(void)SysTick_Config(0U);
		SysTick_CLKSourceConfig(0U);
        ret = DSUCCESS;
    }
    return ret;
}

/******************************************************************************
* Function       :dSysTick_read
* Author         :llemmx
* Date           :2011-05-03
* Description    :读取定时器计数
* Calls          :无
* Table Accessed :无
* Table Updated  :无
* Input          :无
* Output         :无
* Return         :打开成功返回实际读取数据长度，失败返回OPFAULT
*******************************************************************************
* History:        2011-05-03 
*------------------------------------------------------------------------------
* 2011-05-03 : 1.0.0 : llemmx
* Modification   : 创建
*------------------------------------------------------------------------------
******************************************************************************/
int32_t dSysTick_read (uint8_t buf[], uint32_t count)
{
	int32_t ret = OPFAULT;
    if (count >= 4U)
    {
    	uint32_t tmp=lsystick_num;
    	memcpy(&buf[0],&tmp,4U);
    	ret = 4;
    }
    return ret;
}

/******************************************************************************
* Function       :dSysTick_ioctl
* Author         :llemmx
* Date           :2011-05-03
* Description    :读取定时器计数
* Calls          :无
* Table Accessed :无
* Table Updated  :无
* Input          :无
* Output         :无
* Return         :打开成功返回DSUCCESS，失败返回OPFAULT
*******************************************************************************
* History:        2011-05-03 
*------------------------------------------------------------------------------
* 2011-05-03 : 1.0.0 : llemmx
* Modification   : 创建
*------------------------------------------------------------------------------
******************************************************************************/
int32_t dSysTick_ioctl(uint32_t cmd,void* arg)
{
	int32_t ret = OPFAULT;
    TSysTick_ioctl *s=(TSysTick_ioctl *)arg;
    
    if((s!=NULL) && (SYSTICK_IOCTL_REDEF!=cmd))
    {
		(void)SysTick_Config((uint32_t)s->freq);  /*for MISRA-2004*/
		SysTick_CLKSourceConfig(s->mode);/*系统时钟8分频*/
		ret = DSUCCESS;
	}
    return ret;
}
