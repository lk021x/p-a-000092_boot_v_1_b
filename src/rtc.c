/********************************************************************************
 * Copyright (C), 1997-2011, SUNGROW POWER SUPPLY CO., LTD. 
 * File name      :rtc.c
 * Author         :Linfei
 * Date           :2012.07.13
 * Description    :DS1340C驱动，模拟I2C方式，在总线频率为168MHZ时为20KHZ，修改
  *                I2C_delay函数中的i变量值可调节波特率，具体用示波器观察
 * Others         :None
 *-------------------------------------------------------------------------------
 * 2012.07.11 : 1.0.0 : Linfei
 * Modification   : 初始代码编写。
 ********************************************************************************/
#include "rtc.h"
#include <time.h>


// RTC设备地址11010000
#define RTC_ADDRESS                 0xD0

#define RTC_TIME_ADDR     		    0x00	// 秒
#define RTC_CTROL_ADDR              0x07    // 控制寄存器

// 涓流充电控制寄存器
// TCS[3:0]      DS[1,0]     ROUT[1,0]
// TCS为充电开关，只有在1010状态才开启充电
// DS指示VCC和VBACKUP间是否有二极管 10表示有二极管 01表示没有二级管
// ROUT表示所接的电阻是250欧姆(01)、2K欧姆(10)、4K欧姆(11)
#define RTC_CHARGER_ADDR            0x08    // 充放电控制寄存器
#define RTC_FLAG_ADDR               0x09    // 标志寄存器


#define RTC_READ_TRY_TIMES          5

// 判断rtc_ioctl的op参数是否是合法的操作码
#define IS_VALID_RTC_CMD(x)     ((x == RTC_CHARGER_ON_250)   || \
                                 (x == RTC_CHARGER_ON_2K)    || \
                                 (x == RTC_CHARGER_ON_4K)    || \
                                 (x == RTC_CHARGER_ON_250_D) || \
                                 (x == RTC_CHARGER_ON_2K_D)  || \
                                 (x == RTC_CHARGER_ON_4K_D)  || \
                                 (x == RTC_CHARGER_OFF))


int32_t      rtc_opened = FALSE;     //RTC打开标识
time_t   rtc_refTcount = 0;          // 参考时间滴答数(秒数)


static int32_t rtc_checktimer(uint8_t *ctime, uint8_t bcdFlag);

// 延时函数，调节i改变波特率，目前i=550在总线频率为168MHZ时为20KHZ，i=137时为80KHZ，i=221时为50KHZ
void I2C_delay(void)
{	
    u16 i=137; 
    while(i) 
    {
        i--;
    }
}


/********************************************************************************
 * Function       : SendCLK
 * Author         : Xu Shun'an
 * Date           : 2011.05.31
 * Description    : 主机在I2C总线上发送五个脉冲，以便重新获得总线控制权 ,堆栈使用
 *                  了8 bytes 
 * Calls          : I2C_delay
 * Input          : None
 * Output         : None
 * Return         : None 
 *********************************************************************************/
void SendCLK()
{
    //主机释放总线
    SDA_H;

    for (uint8_t i = 1; i < 11; ++i)
    {
        if (i % 2)
        {
            SCL_L;
        }
        else
        {
            SCL_H;
        }

        I2C_delay();
    }
}

/********************************************************************************
 * Function       : I2C_Start
 * Author         : Xu Shun'an
 * Date           : 2011.05.31
 * Description    : 启动I2C操作，此时SCL=1,SDA形成一个下降沿,堆栈使用了4 bytes
 * Calls          : I2C_delay，SendCLK
 * Input          : None
 * Output         : None
 * Return         : TRUE：I2C启动成功； FALSE：I2C启动失败
 *********************************************************************************/
int32_t I2C_Start(void)
{
    SDA_H;

    SCL_H;
    I2C_delay();

    if (!SDA_read )
    {
        SendCLK();
        return FALSE; // SDA线为低电平则总线忙,退出
    }

    SDA_L;
    I2C_delay();

    if (SDA_read)
    {
        SendCLK();
        return FALSE; // SDA线为高电平则总线出错,退出
    }

    SDA_L;
    I2C_delay();

    return TRUE;
}


/********************************************************************************
 * Function       : I2C_Stop
 * Author         : Xu Shun'an
 * Date           : 2011.05.31
 * Description    : 停止I2C操作，此时SCL=1,SDA形成一个上升沿，堆栈使用了8 bytes
 * Calls          : I2C_delay
 * Input          : None
 * Output         : None
 * Return         : None
 *********************************************************************************/
void I2C_Stop(void)
{
    SCL_H;

    SDA_L;
    I2C_delay();

    SDA_H;
    I2C_delay();

    SCL_L;
    I2C_delay();
}


/********************************************************************************
 * Function       : I2C_Ack
 * Author         : Xu Shun'an
 * Date           : 2011.05.31
 * Description    : 主机应答从机,此时SDA=0，SCL形成一个正脉冲，堆栈使用了4 bytes
 * Calls          : I2C_delay
 * Input          : None
 * Output         : None
 * Return         : None
 *********************************************************************************/
void I2C_Ack(void)
{
    SDA_L;

    SCL_L;
    I2C_delay();

    SCL_H;
    I2C_delay();

    SCL_L;
    I2C_delay();
}


/********************************************************************************
 * Function       : I2C_NoAck
 * Author         : Xu Shun'an
 * Date           : 2011.05.31
 * Description    : 主机不应答从机,此时SDA=1，SCL形成一个正脉冲，堆栈使用了4 bytes
 * Calls          : I2C_delay
 * Input          : None
 * Output         : None
 * Return         : None
 *********************************************************************************/
void I2C_NoAck(void)
{
    SDA_H;

    SCL_L;
    I2C_delay();

    SCL_H;
    I2C_delay();

    SCL_L;
    I2C_delay();
}


/********************************************************************************
 * Function       : I2C_WaitAck
 * Author         : Xu Shun'an
 * Date           : 2011.05.31
 * Description    : 主机非阻塞式监听从机发来的应答信号,时序:SCL=1,SDA=0，堆栈使用
 *                  了4 bytes
 * Calls          : I2C_delay
 * Input          : None
 * Output         : None
 * Return         : TRUE：成功监听到，FALSE：未能监听到
 *********************************************************************************/
int32_t I2C_WaitAck(void)
{
    //uint16_t i = 1000;

    //主机释放总线
    SDA_H;

    SCL_L;
    I2C_delay();

    SCL_H;
    I2C_delay();

    while (SDA_read)
    {
        SCL_L;
        return FALSE;
    }

    SCL_L;
    I2C_delay();

    return TRUE;
}


/********************************************************************************
 * Function       : I2C_SendByte
 * Author         : Xu Shun'an
 * Date           : 2011.05.31
 * Description    : 主机向从机发送一个字节,数据从高位到低位；时序:SCL上升沿时将SDA
 *                  数据发送给从机，堆栈使用了16 bytes
 * Calls          : I2C_delay
 * Input          : None
 * Output         : None
 * Return         : None
 *********************************************************************************/
void I2C_SendByte(u8 SendByte)
{
    uint8_t i = 8;

    SCL_L;
    I2C_delay();

    while (i--)
    {
        if (SendByte &0x80)
        {
            SDA_H;
        }
        else
        {
            SDA_L;
        }

        I2C_delay();

        SendByte <<= 1;

        SCL_H;
        I2C_delay();

        SCL_L;
        I2C_delay();
    }
}


/*********************************************************************************
 * Function       : I2C_SendByteWithAck
 * Author         : Xu Shun'an
 * Date           : 2011.05.31
 * Description    : 主机向从机发送一个字节,数据从高位到低位,但此时主机需要从机应答
 *                  堆栈使用了4 bytes
 * Calls          : I2C_WaitAck，I2C_SendByte，I2C_Stop
 * Input          : None
 * Output         : None
 * Return         : TRUE:主机发送数据后收到从机应答；FALSE:主机发送数据后未收到应答
 **********************************************************************************/
int32_t I2C_SendByteWithAck(u8 SendByte)
{

    I2C_SendByte(SendByte);

    if (!I2C_WaitAck())
    {
        I2C_Stop();
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


/*********************************************************************************
 * Function       : I2C_ReceiveByte
 * Author         : Xu Shun'an
 * Date           : 2011.05.31
 * Description    : 主机读取从机发来的一个字节,数据从高位到低位.时序:SCL=1,SDA读出
 *                  堆栈使用了12 bytes
 * Calls          : I2C_delay
 * Input          : None
 * Output         : None
 * Return         : ReceiveByte:主机读取的字节数据
 **********************************************************************************/
uint8_t I2C_ReceiveByte(void)
{
    uint8_t i = 8;
    uint8_t ReceiveByte = 0;

    //主机释放总线
    SDA_H;
    SCL_L;
    I2C_delay();

    while (i--)
    {
        ReceiveByte <<= 1;

        SCL_H;
        I2C_delay();

        if (SDA_read)
        {
            ReceiveByte |= 0x01;
        }

        SCL_L;
        I2C_delay();
    }

    return ReceiveByte;
}


/**********************************************************************************
 * Function       : I2C_WriteByte
 * Author         : Linfei
 * Date           : 2012.07.11
 * Description    : 主机向从机写入一个字节的数据
 * Calls          : I2C_Start，I2C_SendByteWithAck，I2C_Stop
 * Input          : dev:设备地址；
 *                  addr:待写入数据的地址，低8位有效
 *　　　　　　　　　byte:待写入字节的地址
 * Output         : None
 * Return         : TRUE：写入成功；FALSE：I2C未开启、设备未应答、地址错误，写入失败
 ***********************************************************************************/
int32_t I2C_WriteByte(uint8_t dev, uint16_t addr, uint8_t *byte)
{
    //开启I2C操作
    if (!I2C_Start())
    {
        return FALSE;
    }

    //发送设备号
    if (!I2C_SendByteWithAck(dev & 0xFE))
    {
        return FALSE;
    }
    
    // 发送高8位地址
//    if (!I2C_SendByteWithAck((uint8_t)((addr >> 8) & 0xFF)))
//    {
//        return FALSE;
//    }

    // 发送低8位地址
    if (!I2C_SendByteWithAck((uint8_t)(addr & 0xFF)))
    {
        return FALSE;
    }

    if(byte != NULL)
    {
        // 发送内容
        if (!I2C_SendByteWithAck(*byte))
        {
            return FALSE;
        }
    }
    
    // 结束操作
    I2C_Stop();
    return TRUE;
}


/**********************************************************************************
 * Function       : I2C_ReadByte
 * Author         : Linfei
 * Date           : 2012.07.11
 * Description    : 主机从从机读取一个字节的数据，数据地址由I2C_WriteByte操作提供
 * Calls          : I2C_Start，I2C_SendByteWithAck，I2C_Stop，I2C_ReceiveByte
 * Input          : dev:设备地址；
 *　　　　　　　　　byte:存放读出字节的地址
 * Output         : None
 * Return         : TRUE：读取成功；FALSE：I2C未开启、设备未应答、地址错误，读取失败
 ***********************************************************************************/
int32_t I2C_ReadByte(uint8_t dev, uint8_t *byte)
{
    // 开启I2C操作
    if (!I2C_Start())
    {
        return FALSE;
    }
    
    
    // 发送设备号和读操作码
    if (!I2C_SendByteWithAck(dev | 0x01))
    {
        return FALSE;
    }
    
    *byte = I2C_ReceiveByte();
    I2C_NoAck();

    //结束读操作
    I2C_Stop();

    return TRUE;
}


/**********************************************************************************
 * Function       : rtc_open
 * Author         : Linfei
 * Date           : 2012.07.13
 * Description    : 打开RTC时钟，初始化开启内部晶振（防止某些情况下晶振停止，详见手册）
 * Calls          : I2C_WriteByte，I2C_ReadByte
 * Input          : flag:未使用
 *　　　　　　　　mode:未使用
 * Output         : None
 * Return         : EFAULT：开启失败；
 *                  DSUCCESS：开启成功
 ***********************************************************************************/
int32_t rtc_open(int32_t flag, int32_t mode)
{
    if(rtc_opened == TRUE)
    {
        return EFAULT;
    }
    
    // 用户设定的参考时间合法性检查
    uint8_t time[6] = { REF_YEAR, REF_MONTH, REF_DATE, 0x00, 0x00, 0x00};
    if(rtc_checktimer(time, 1) != TRUE)
    {
        return EFAULT;
    }
    
    // 清除晶振停止标志
    uint8_t cmd = 0;
    if(I2C_WriteByte(RTC_ADDRESS, RTC_FLAG_ADDR, &cmd) != TRUE)  
    {
        return EFAULT;
    }

    struct tm tmpTime = 
    {
        .tm_year = ((time[0] >> 4) & 0x0F)*10 + (time[0] & 0x0F) + 100,
        .tm_mon  = ((time[1] >> 4) & 0x0F)*10 + (time[1] & 0x0F) - 1,
        .tm_mday = ((time[2] >> 4) & 0x0F)*10 + (time[2] & 0x0F),
        .tm_hour = ((time[3] >> 4) & 0x0F)*10 + (time[3] & 0x0F),
        .tm_min  = ((time[4] >> 4) & 0x0F)*10 + (time[4] & 0x0F),
        .tm_sec  = ((time[5] >> 4) & 0x0F)*10 + (time[5] & 0x0F),
        .tm_isdst= 0,
    };
    rtc_refTcount = mktime(&tmpTime);
    
    if(rtc_refTcount == 0xFFFFFFFF)
    {
        return EFAULT;
    }
   
    rtc_opened = TRUE;
    return DSUCCESS;        
}

// 读取RTC时间
int32_t rtc_timeRead(uint8_t *time)
{
    // 设置寄存器指针为0
    if(I2C_WriteByte(RTC_ADDRESS, 0, NULL) != TRUE)
    {
        return FALSE;
    }

    // 读取时间
    for(uint8_t i = 0; i < 7; i++)
    {
        if(I2C_ReadByte(RTC_ADDRESS, &time[i]) != TRUE)
        {
            return FALSE;
        }
    }
    
    time[0] &= 0x7F;
    time[1] &= 0x7F;
    time[2] &= 0x3F;
    time[4] &= 0x3F;
    time[5] &= 0x1F;
    return TRUE;
}



/**********************************************************************************
 * Function       : rtc_read
 * Author         : Linfei
 * Date           : 2012.07.13
 * Description    : 读取RTC时间，顺序为：年 月 日 时 分 秒
 * Calls          : I2C_WriteByte，I2C_ReadByte
 * Input          : buf:保存待读取出的时间
 *　　　　　　　  : count:待读取出的时间数组长度，固定为6
 * Output         : None
 * Return         : EFAULT：读取失败
 *                  DSUCCESS：读取成功
 ***********************************************************************************/
int32_t rtc_read(char *buf, uint32_t count)
{
    if(rtc_opened == FALSE || buf == NULL || count != 6)
    {
        return EFAULT;
    }
    
    uint8_t time[7] = { 0 };
    uint8_t rTry = RTC_READ_TRY_TIMES;
    while(rTry--)
    {
        // 读取时间
        if(rtc_timeRead(time) != TRUE)
        {
            return EFAULT;
        }

        // 合法性判断，与参考时间对比
        struct tm readTime =
        {
            .tm_year = ((time[6] >> 4) & 0x0F)*10 + (time[6] & 0x0F) + 100,
            .tm_mon  = ((time[5] >> 4) & 0x0F)*10 + (time[5] & 0x0F) - 1,
            .tm_mday = ((time[4] >> 4) & 0x0F)*10 + (time[4] & 0x0F),
            .tm_hour = ((time[2] >> 4) & 0x0F)*10 + (time[2] & 0x0F),
            .tm_min  = ((time[1] >> 4) & 0x0F)*10 + (time[1] & 0x0F),
            .tm_sec  = ((time[0] >> 4) & 0x0F)*10 + (time[0] & 0x0F),
            .tm_isdst= 0,
        };
        time_t checkTcount = mktime(&readTime);
        if(checkTcount > rtc_refTcount)
        {
            // 按年月日时分秒排序
            buf[0] = time[6];
            buf[1] = time[5];
            buf[2] = time[4];
            buf[3] = time[2];
            buf[4] = time[1];
            buf[5] = time[0];
            return DSUCCESS;
        }
    }
    return EFAULT;
}



// 时间合法性判断，顺序：年月日时分秒，支持BCD码和二进制
static int32_t rtc_checktimer(uint8_t *ctime, uint8_t bcdFlag)
{
    uint8_t tmpTime[6];
    uint8_t *time = ctime;
    if(bcdFlag == 1)
    {
        tmpTime[0] = ((ctime[0]>>4) & 0x0F)*10 + (ctime[0] & 0x0F);
        tmpTime[1] = ((ctime[1]>>4) & 0x0F)*10 + (ctime[1] & 0x0F);
        tmpTime[2] = ((ctime[2]>>4) & 0x0F)*10 + (ctime[2] & 0x0F);
        tmpTime[3] = ((ctime[3]>>4) & 0x0F)*10 + (ctime[3] & 0x0F);
        tmpTime[4] = ((ctime[4]>>4) & 0x0F)*10 + (ctime[4] & 0x0F);
        tmpTime[5] = ((ctime[5]>>4) & 0x0F)*10 + (ctime[5] & 0x0F);
        time = tmpTime;
    }

    if(time[0] > 99)                   //0~99
    {
        return FALSE;
    }
    if((time[1] > 12) || (time[1] == 0))     //1~12
    {
        return FALSE;
    }
    if(time[2] == 0)                   //1~31
    {
        return FALSE;
    }
    //月大31天，2月正常28天，遇润年29天，其余月小30天
    if (time[1] == 1 || time[1] == 3 || \
            time[1] == 5 || time[1] == 7 || \
            time[1] == 8 || time[1] == 10 || \
            time[1] == 12)
    {
        if (time[1] > 31)
        {
            return FALSE;
        }
    }

    if (time[1] == 4 || time[1] == 6 || \
            time[1] == 9 || time[1] == 11)
    {
        if (time[2] > 30)
        {
            return FALSE;
        }
    }

    if (time[1] == 2)
    {
        if (time[0] % 4 == 0 && time[2] > 29)
        {
            return FALSE;
        }
        if (time[0] % 4 != 0 && time[2] > 28)
        {
            return FALSE;
        }
    }
    if(time[3] > 23)                   //0~23
    {
        return FALSE;
    }
    if(time[4] > 59)                   //0~59
    {
        return FALSE;
    }
    if(time[5] > 59)                   //0~59
    {
        return FALSE;
    }
    return TRUE;
}

/**********************************************************************************
 * Function       : rtc_write
 * Author         : Linfei
 * Date           : 2012.07.11
 * Description    : 设定RTC时间，顺序为：年 月 日 时 分 秒
 * Calls          : I2C_WriteByte
 * Input          : buf:保存待设置的时间
 *　　　　　　　  : count:待设置时间数组长度，固定为6
 * Output         : None
 * Return         : EFAULT：设置失败
 *                  DSUCCESS：设置成功
 ***********************************************************************************/
int32_t rtc_write(const char *buf, uint32_t count)
{
    uint32_t i = 0;
    if(rtc_opened == FALSE || buf == NULL || count != 6)
    {
        return EFAULT;
    }
    
    if(rtc_checktimer((uint8_t *)buf, 1) != TRUE)
    {
        return EFAULT;
    }

    uint8_t time[7] = { buf[5], buf[4], buf[3], 1, buf[2], buf[1], buf[0] };
    
    for(i = 0; i < 7; i++)
    {
        if(I2C_WriteByte(RTC_ADDRESS, i, (uint8_t *)&time[i]) != TRUE)
        {
            return EFAULT;
        }
    }
    return DSUCCESS;
}


/**********************************************************************************
 * Function       : rtc_ioctl
 * Author         : Linfei
 * Date           : 2012.07.11
 * Description    : RTC控制，设置开启或关闭涓流充电
 * Calls          : I2C_WriteByte
 * Input          : op:操作码，目前支持的是RTC_CHARGER_ON，RTC_CHARGER_OFF
 *　　　　　　　  : arg:用户参数，暂时未用
 * Output         : None
 * Return         : EFAULT：设置失败
 *                  DSUCCESS：设置成功
 ***********************************************************************************/
int32_t rtc_ioctl(uint32_t op, void *arg)
{
    if(rtc_opened == FALSE)
    {
        return EFAULT;
    }
    
    uint8_t cmd = (uint8_t)op;
    switch(cmd)
    {
        case RTC_CHARGER_OFF:
        case RTC_CHARGER_ON_250:
        case RTC_CHARGER_ON_4K_D:
        case RTC_CHARGER_ON_2K:
        case RTC_CHARGER_ON_4K:
        case RTC_CHARGER_ON_250_D:
        case RTC_CHARGER_ON_2K_D:
            if(I2C_WriteByte(RTC_ADDRESS, RTC_CHARGER_ADDR, &cmd) != TRUE)
            {
               return EFAULT;
            }
            break;
        default:
            return EFAULT;
    }

// for test
//    uint8_t reg = 0;
//    I2C_WriteByte(RTC_ADDRESS, RTC_CHARGER_ADDR, NULL);
//    I2C_ReadByte(RTC_ADDRESS, &reg);
    return DSUCCESS;
}

/**********************************************************************************
 * Function       : rtc_release
 * Author         : Linfei
 * Date           : 2012.07.12
 * Description    : RTC程序关闭
 * Calls          : None
 * Input          : None
 * Output         : None
 * Return         : DSUCCESS：设置成功
 ***********************************************************************************/

int32_t rtc_release(void)
{
    if(rtc_opened == TRUE)
    {
        return DSUCCESS;
    }
    rtc_opened = FALSE;
    return DSUCCESS;
}

