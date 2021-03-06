/********************************************************************************
 * Copyright (C), 1997-2011, SUNGROW POWER SUPPLY CO., LTD. 
 * File name      :extwdg.c
 * Author         :Linfei
 * Date           :2012.08.30
 * Description    :外部看门狗TPS3823驱动，喂狗脉冲2us，超时复位时间固定为1.6s
 * Others         :None
 *-------------------------------------------------------------------------------
 * 2012.08.30 : 1.0.0 : Linfei
 * Modification   : 初始代码编写。
 ********************************************************************************/
#include "extwdg.h"

bool extwdg_opened = false;


/* 内部函数声明 */
void extwdg_delay(uint32_t nCount);



/*******************************************************************************
* Function Name  : Delay
* Description    : Inserts a delay time.
* Input          : nCount: specifies the delay time length.
* Output         : None
* Return         : None
*******************************************************************************/
void extwdg_delay(uint32_t nCount)
{
    while(nCount > 0U)
    {
    	nCount--;
    }
}


/*******************************************************************************
 * Function       : extwdg_open
 * Author         : Linfei
 * Date           : 2012.08.30
 * Description    : 打开外部看门狗
 * Calls          : None
 * Input          : 无具体意义，可都为0
 * Output         : None
 * Return         : DSUCCESS：打开成功 
 ********************************************************************************
 *-------------------------------------------------------------------------------
 * 2012.08.30 : 1.0.0 : Linfei
 * Modification   : 初始代码编写
 ********************************************************************************/
int32_t extwdg_open(int32_t flag, int32_t mode)
{
    int32_t ret = OPFAULT;
    if(extwdg_opened != true)
    {
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Pin = EWDG_Pin;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(EWDG_GROUP, &GPIO_InitStructure); 
        
        GPIO_ResetBits(EWDG_GROUP, EWDG_Pin);
        
        extwdg_opened = true;
        ret = DSUCCESS;
    }
    return ret;
}



/*******************************************************************************
 * Function       : extwdg_ioctl
 * Author         : Linfei
 * Date           : 2012.08.30
 * Description    : 喂狗
 * Calls          : None
 * Input          : op：操作码：喂狗
 *                  arg：未用
 * Output         : None
 * Return         : DSUCCESS: 设置成功；EFAULT：设置失败
 ********************************************************************************
 *-------------------------------------------------------------------------------
 * 2012.08.30 : 1.0.0 : Linfei
 * Modification   : 初始代码编写
 ********************************************************************************/
int32_t extwdg_ioctl(uint32_t op, void *arg)
{
    int32_t ret = OPFAULT;
    arg = arg;
    if((extwdg_opened == true) && (op == FEED_EXTWDG))
    {
        GPIO_SetBits(EWDG_GROUP, EWDG_Pin);
        extwdg_delay(80U);
        GPIO_ResetBits(EWDG_GROUP, EWDG_Pin);
        extwdg_delay(80U);
        ret = DSUCCESS;
    }
    return ret;
}

/*******************************************************************************
 * Function       : extwdg_close
 * Author         : Linfei
 * Date           : 2012.08.30
 * Description    : 关闭外部看门狗
 * Calls          : None
 * Input          : None
 * Output         : None
 * Return         : DSUCCESS：关闭成功 
 ********************************************************************************
 *-------------------------------------------------------------------------------
 * 2012.08.30 : 1.0.0 : Linfei
 * Modification   : 初始代码编写
 ********************************************************************************/
int32_t extwdg_close(void)
{
    if (extwdg_opened == true)
    {
        /* Deinit external watchdog */
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Pin = EWDG_Pin;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(EWDG_GROUP, &GPIO_InitStructure); 

        extwdg_opened = false;
    }
    return DSUCCESS;    
}

