/********************************************************************************
 * Copyright (C), 1997-2011, SUNGROW POWER SUPPLY CO., LTD. 
 * File name      :EEPROM.c
 * Author         :Linfei
 * Date           :2012.08.11
 * Description    :EEPROM驱动，适用于STM32F4x系列芯片，支持页写和页等待且可配置，
 *                 定义EEPROM操作函数
 * Others         :None
 *-------------------------------------------------------------------------------
 * 2012.08.11 : 1.0.0 : Linfei
 * Modification   : 初始代码编写。
 ********************************************************************************/

#include "EEPROM.h"



/* EEPROM设置区 ------------------------------------------------------------------- */

/* EEPROM功能码定义 */
#define EEPROM_WREN                 0x06U
#define EEPROM_WRDI                 0x04U
#define EEPROM_RDSR                 0x05U
#define EEPROM_WRSR                 0x01U
#define EEPROM_READ                 0x03U
#define EEPROM_WRITE                0x02U

#define DummyByte                   0xA5U

#define WRITE_STATUS_MASK           0x01U


static bool eeprom_opened = false;         /* EEPROM打开标识 */
static uint32_t eeprom_lptr = 0U;              /*全局文件地址指针 */

#ifdef EEPROM_WRITE_WAIT
volatile uint16_t eeprom_wait_count = 0U;        /* EEPROM写等待计数 */
#endif

/* 内部函数声明 */
int32_t EEPROM_init(void);
uint8_t SPI_EEPROM_SendByte(uint8_t byte);
uint8_t SPI_EEPROM_ReadByte(void);
void SPI_EEPROM_WriteEnable(void);
bool SPI_EEPROM_WaitForWriteEnd(void);
bool SPI_EEPROM_Write(const uint8_t data[], uint32_t address, uint32_t length);
bool IS_VALID_EEPROM_PROCODE(uint32_t procode);





/*******************************************************************************
 * Function       : IS_VALID_EEPROM_PROCODE
 * Author         : Linfei
 * Date           : 2012.09.13
 * Description    : 判断是否是合法的写保护码
 * Calls          : None
 * Input          : None
 * Output         : None
 * Return         : None
 ********************************************************************************
 *-------------------------------------------------------------------------------
 * 2012.09.13 : 1.0.0 : Linfei
 * Modification   : 初始代码编写
 ********************************************************************************/
bool IS_VALID_EEPROM_PROCODE(uint32_t procode)
{
    bool ret = false;
    if((procode == EEPROM_PRO_NONE) ||
       (procode == EEPROM_PRO_UPER_QUARTER) ||
       (procode == EEPROM_PRO_UPER_HALF) ||
       (procode == EEPROM_PRO_ALL))
    {
        ret = true;
    }
    return ret;
}


/*******************************************************************************
 * Function       : EEPROM_init
 * Author         : Linfei
 * Date           : 2012.09.06
 * Description    : EEPROM管脚初始化
 * Calls          : None
 * Input          : None
 * Output         : None
 * Return         : None
 ********************************************************************************
 *-------------------------------------------------------------------------------
 * 2012.09.06 : 1.0.0 : Linfei
 * Modification   : 初始代码编写
 ********************************************************************************/
int32_t EEPROM_init(void)
{
    SPI_InitTypeDef  SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    EEPROM_SPI_RCC_OPEN;

    /* Configure SPI pins: SCK, MISO and MOSI */
    GPIO_InitStructure.GPIO_Pin = EEPROM_MISO_Pin;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(EEPROM_MISO_GROUP, &GPIO_InitStructure);

    GPIO_PinAFConfig(EEPROM_MISO_GROUP, EEPROM_MISO_PinSource, EEPROM_SPI_AF_PORT);

    GPIO_InitStructure.GPIO_Pin = EEPROM_MOSI_Pin;
    GPIO_Init(EEPROM_MOSI_GROUP, &GPIO_InitStructure);
    GPIO_PinAFConfig(EEPROM_MOSI_GROUP, EEPROM_MOSI_PinSource, EEPROM_SPI_AF_PORT);

    GPIO_InitStructure.GPIO_Pin = EEPROM_SCK_Pin;
    GPIO_Init(EEPROM_SCK_GROUP, &GPIO_InitStructure);
    GPIO_PinAFConfig(EEPROM_SCK_GROUP, EEPROM_SCK_PinSource, EEPROM_SPI_AF_PORT);

    /* Configure I/O for EEPROM Chip select */
    GPIO_InitStructure.GPIO_Pin = EEPROM_NSS_Pin;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(EEPROM_NSS_GROUP, &GPIO_InitStructure);

    /* Deselect the EEPROM: Chip Select high */
    SPI_EEPROM_CS_HIGH;

    /* SPI configuration Mode 3*/
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;   /* SCK空闲时高电平 */
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;  /* 第二个时钟边沿采样 */
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7U;
    SPI_Init(EEPROM_SPI_PORT, &SPI_InitStructure);

    /* Enable SPI  */
    SPI_Cmd(EEPROM_SPI_PORT, ENABLE);

    /* 整个EEPROM地址范围取消写保护 */
    return EEPROM_ioctl(EEPROM_PRO_NONE, NULL);
}




/*******************************************************************************
* Function Name  : SPI_EEPROM_SendByte
* Description    : Sends a byte through the SPI interface and return the byte
*                  received from the SPI bus.
* Input          : byte : byte to send.
* Output         : None
* Return         : The value of the received byte.
*******************************************************************************/
uint8_t SPI_EEPROM_SendByte(uint8_t byte)
{
    uint16_t sendbyte = byte;
    /* Loop while DR register in not empty */
    while (SPI_I2S_GetFlagStatus(EEPROM_SPI_PORT, SPI_I2S_FLAG_TXE) == RESET)
    {

    }

    /* Send byte through the SPI peripheral */
    SPI_I2S_SendData(EEPROM_SPI_PORT, sendbyte);

    /* Wait to receive a byte */
    while (SPI_I2S_GetFlagStatus(EEPROM_SPI_PORT, SPI_I2S_FLAG_RXNE) == RESET)
    {

    }

    /* Return the byte read from the SPI bus */
    return (uint8_t)SPI_I2S_ReceiveData(EEPROM_SPI_PORT);
}

/*******************************************************************************
* Function Name  : SPI_EEPROM_ReadByte
* Description    : Reads a byte from the SPI EEPROM.
*                  This function must be used only if the Start_Read_Sequence
*                  function has been previously called.
* Input          : None
* Output         : None
* Return         : Byte Read from the SPI EEPROM.
*******************************************************************************/
uint8_t SPI_EEPROM_ReadByte(void)
{
    return (SPI_EEPROM_SendByte(DummyByte));
}


/*******************************************************************************
* Function Name  : SPI_EEPROM_WriteEnable
* Description    : Enables the write access to the EEPROM.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_EEPROM_WriteEnable(void)
{
    SPI_EEPROM_CS_LOW;

    uint8_t ret = SPI_EEPROM_SendByte(EEPROM_WREN);

    SPI_EEPROM_CS_HIGH;
}

/*******************************************************************************
* Function Name  : SPI_EEPROM_WaitForWriteEnd
* Description    : Polls the status of the Write In Progress (WIP) flag in the
*                  FLASH's status  register  and  loop  until write  opertaion
*                  has completed.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool SPI_EEPROM_WaitForWriteEnd(void)
{
    bool end = FALSE;
    uint8_t EEPROM_Status = 0U;

    /* Select the FLASH: Chip Select low */
    SPI_EEPROM_CS_LOW;

    /* Send "Read Status Register" instruction */
    uint8_t ret = SPI_EEPROM_SendByte(EEPROM_RDSR);

    uint32_t count = 600U;
    /* Loop as long as the memory is busy with a write cycle */
    do
    {
        /* Send a dummy byte to generate the clock needed by the EEPROM
        and put the value of the status register in EEPROM_Status variable */
        EEPROM_Status = SPI_EEPROM_SendByte(DummyByte);
        count--;
    }
    while(((EEPROM_Status & WRITE_STATUS_MASK) == (uint8_t)SET) && (count != 0)); /* Write in progress */
    if(count != 0)
    {
        end = TRUE;
    }
    /* Deselect the FLASH: Chip Select high */
    SPI_EEPROM_CS_HIGH;
    return end;
}


/*******************************************************************************
 * Function       : eeprom_write_wait_count
 * Author         : Linfei
 * Date           : 2012.09.06
 * Description    : EEPROM写等待计数
 * Calls          : None
 * Input          : None
 * Output         : None
 * Return         : None
 ********************************************************************************
 *-------------------------------------------------------------------------------
 * 2012.09.06 : 1.0.0 : Linfei
 * Modification   : 初始代码编写
 ********************************************************************************/
void eeprom_write_wait_count(void)
{
#ifdef EEPROM_WRITE_WAIT
    if(eeprom_wait_count > 0U)
    {
        eeprom_wait_count--;
    }
#endif
}

/*******************************************************************************
 * Function       : SPI_EEPROM_Write
 * Author         : Linfei
 * Date           : 2012.09.06
 * Description    : EEPROM写操作
 * Calls          : None
 * Input          : data: 待写入数据的首地址
 *                  address: 数据写入到EEPROM的首地址
 *                  length: 写入的字节数
 * Output         : None
 * Return         : None
 ********************************************************************************
 *-------------------------------------------------------------------------------
 * 2012.09.06 : 1.0.0 : Linfei
 * Modification   : 初始代码编写
 ********************************************************************************/
bool SPI_EEPROM_Write(const uint8_t data[], uint32_t address, uint32_t length)
{
    bool ret = FALSE;
    SPI_EEPROM_WriteEnable();

    SPI_EEPROM_CS_LOW;

    (void)SPI_EEPROM_SendByte(EEPROM_WRITE);
    (void)SPI_EEPROM_SendByte((uint8_t)((address & 0xFF00U) >> 8));
    (void)SPI_EEPROM_SendByte((uint8_t)(address & 0xFFU));

    for(uint32_t i = 0U; i < length; i++)
    {
        (void)SPI_EEPROM_SendByte(data[i]);
    }
    SPI_EEPROM_CS_HIGH;

    /* 等待写结束 */
#ifdef EEPROM_WRITE_WAIT    
    eeprom_wait_count = EEPROM_WAIT_TIME;
    while(eeprom_wait_count != 0U)
    {
    }
#endif


    ret = TRUE;

#ifdef EEPROM_WRITE_WAIT
    if(SPI_EEPROM_WaitForWriteEnd() == FALSE)
    {
        ret = FALSE;
    }
#endif
    return ret;
}


#ifdef EEPROM_WRITE_BYPAGE
/*******************************************************************************
 * Function       : EEPROM_write_bytes_by_page
 * Author         : Linfei
 * Date           : 2012.09.06
 * Description    : EEPROM按页写操作
 * Calls          : None
 * Input          : data: 待写入数据的首地址
 *                  address: 数据写入到EEPROM的首地址
 *                  length: 写入的字节数
 * Output         : None
 * Return         : None
 ********************************************************************************
 *-------------------------------------------------------------------------------
 * 2012.09.06 : 1.0.0 : Linfei
 * Modification   : 初始代码编写
 ********************************************************************************/
void EEPROM_write_bytes_by_page(uint8_t* data, uint32_t address, uint32_t length)
{
    uint32_t NumOfPage = 0;
    uint32_t NumOfSingle = 0;
    uint32_t Addr = 0;
    uint32_t count = 0;
    
    Addr = address % SPI_EEPROM_PageSize;
    count = SPI_EEPROM_PageSize - Addr;
    NumOfPage =  length / SPI_EEPROM_PageSize;
    NumOfSingle = length % SPI_EEPROM_PageSize;

    if ((address % SPI_EEPROM_PageSize) == 0) /* 待写入数据的起始地址为页对齐 */
    {
        if (NumOfPage == 0) /* 待写数据个数小于一页 */
        {
            SPI_EEPROM_Write(data, address, length);
        }
        else /* 待写数据个数大于一页 */
        {
            while(NumOfPage--)
            {
                SPI_EEPROM_Write(data, address, SPI_EEPROM_PageSize);
                address += SPI_EEPROM_PageSize;
                data += SPI_EEPROM_PageSize;
            }

            /* 写剩余不满一页数据 */
            if(NumOfSingle != 0)
            {
                SPI_EEPROM_Write(data, address, NumOfSingle);
            }
        }
    }
    else /* 待写入数据的起始地址非页对齐 */
    {
        if (NumOfPage == 0)  /* 待写数据个数小于一页 */
        {
            if (NumOfSingle > count) /* 待写数据跨页，分两次写 */
            {
                SPI_EEPROM_Write(data, address, count);
                address += count;
                data += count;

                SPI_EEPROM_Write(data, address, NumOfSingle - count);
            }
            else
            {
                SPI_EEPROM_Write(data, address, length);
            }
        }
        else /* 待写数据个数多于一页 */
        {
            SPI_EEPROM_Write(data, address, count);
            address += count;
            data += count;

            length -= count;
            NumOfPage =  length / SPI_EEPROM_PageSize;
            NumOfSingle = length % SPI_EEPROM_PageSize;

            /* 写整页数据 */
            while(NumOfPage--)
            {
                SPI_EEPROM_Write(data, address, SPI_EEPROM_PageSize);
                address +=  SPI_EEPROM_PageSize;
                data += SPI_EEPROM_PageSize;
            }

            /* 写剩余不满一页数据 */
            if (NumOfSingle != 0)
            {
                SPI_EEPROM_Write(data, address, NumOfSingle);
            }
        }
    }
}
#endif


/*******************************************************************************
 * Function       : EEPROM_open
 * Author         : Linfei
 * Date           : 2012.08.11
 * Description    : 打开并初始化EEPROM设备
 * Calls          : spi_init
 * Input          : 无具体意义，可都为0
 * Output         : None
 * Return         : DSUCCESS：打开成功 
 ********************************************************************************
 *-------------------------------------------------------------------------------
 * 2012.08.11 : 1.0.0 : Linfei
 * Modification   : 初始代码编写
 ********************************************************************************/
int32_t EEPROM_open(int32_t flag, int32_t mode)
{
    int32_t ret = OPFAULT;
    if(eeprom_opened != TRUE)
    {
        eeprom_opened = TRUE;
        if(EEPROM_init() == DSUCCESS)
        {
            ret = DSUCCESS;
        }
        else
        {
            eeprom_opened = FALSE;
        }
    }
    return ret;
}

/*******************************************************************************
 * Function       : EEPROM_write
 * Author         : Linfei
 * Date           : 2012.08.11
 * Description    : 向EEPROM中顺序写入数据，若地址达到EEPROM最大地址，则停止写入
 * Calls          : None
 * Input          : buf: 待写入数据存储地址指针；count：待写入数据字节长度
 * Output         : None
 * Return         : OPFAULT：写入失败； count：实际所写入的字节数
 ********************************************************************************
 *-------------------------------------------------------------------------------
 * 2012.08.11 : 1.0.0 : Linfei
 * Modification   : 初始代码编写
 ********************************************************************************/
int32_t EEPROM_write(const uint8_t buf[], uint32_t count)
{
    int32_t ret = OPFAULT;
    if ((eeprom_opened == true) && (buf != NULL))
    {
        /*判断要写入的内容是否超过了文件末尾 */
        if ((eeprom_lptr + count) >= (uint32_t)EEPROM_SIZE)
        {
            count = (uint32_t)EEPROM_SIZE - 1U - eeprom_lptr; /*如果超过,则只读到文件末尾 */
        }

        if(count != 0U)
        {
        #ifdef EEPROM_WRITE_BYPAGE
            EEPROM_write_bytes_by_page((uint8_t *)buf, eeprom_lptr, count);
        #else
            SPI_EEPROM_Write(buf, eeprom_lptr, count);
        #endif

            eeprom_lptr += count;
        }
        ret = (int32_t)count;
    }
    return ret;
}

/*******************************************************************************
 * Function       : EEPROM_read
 * Author         : Linfei
 * Date           : 2012.08.11
 * Description    : 顺序读取EEPROM中数据，若请求的数据地址达到EEPROM最大地址则
 *                  终止读取
 * Calls          : None
 * Input          : buf: 待读取数据存储地址指针；count：待读取数据字节长度
 * Output         : None
 * Return         : OPFAULT：读取失败； count：实际所读取到的字节数
 ********************************************************************************
 *-------------------------------------------------------------------------------
 * 2012.08.11 : 1.0.0 : Linfei
 * Modification   : 初始代码编写
 ********************************************************************************/
int32_t EEPROM_read(uint8_t buf[], uint32_t count)
{
    int32_t ret = OPFAULT;
    if ((eeprom_opened == true) && (buf != NULL))
    {
        /* 判断要读取的内容是否超过了文件末尾 */
        if ((eeprom_lptr + count) >= (uint32_t)EEPROM_SIZE)
        {
            count = (uint32_t)EEPROM_SIZE - 1U - eeprom_lptr; /* 如果超过,则只读到文件末尾 */
        }

        if(count != 0U)
        {
            SPI_EEPROM_CS_LOW;
            (void)SPI_EEPROM_SendByte(EEPROM_READ);
            (void)SPI_EEPROM_SendByte((eeprom_lptr & 0xFF00U) >> 8);
            (void)SPI_EEPROM_SendByte(eeprom_lptr & 0xFFU);

            for(uint32_t i = 0U; i < count; i++)
            {
                buf[i] = SPI_EEPROM_ReadByte();
            }

            SPI_EEPROM_CS_HIGH;

            eeprom_lptr += count;
        }
        ret = (int32_t)count;
    }
    return ret;
}



/*******************************************************************************
 * Function       : EEPROM_lseek
 * Author         : Linfei
 * Date           : 2012.08.11
 * Description    : 设置全局文件指针
 * Calls          : None
 * Input          : offset:指针偏移量，offset>0标识向前偏移，offset<0表示向后偏移
 *                  where: 当前指针指向文件的地方，共有三种取值方法：
 *                  SEEK_SET-文件开始处，SEEK_CUR-当前文件处，SEEK_END-文件结尾处
 * Output         : None
 * Return         : DSUCCESS: 设置成功；OPFAULT：设置失败
 ********************************************************************************
 *-------------------------------------------------------------------------------
 * 2012.08.11 : 1.0.0 : Linfei
 * Modification   : 初始代码编写
 ********************************************************************************/
int32_t EEPROM_lseek(int64_t offset, int32_t where)
{
    int32_t ret = OPFAULT;
    if(eeprom_opened == true)
    {
        switch(where)
        {
        case SEEK_SET:
            if ((offset >= 0) && (offset < EEPROM_SIZE))
            {
                eeprom_lptr = (uint32_t)offset;
                ret = DSUCCESS;
            }
            break;

        case SEEK_END:
            if ((offset <= 0) && (offset > -EEPROM_SIZE))
            {
                offset = -offset;
                eeprom_lptr = ((uint32_t)EEPROM_SIZE - 1U) - (uint32_t)offset;
                ret = DSUCCESS;
            }
            break;

        case SEEK_CUR:
            if((((int64_t)eeprom_lptr + offset) >= 0) && (((int64_t)eeprom_lptr + offset) < EEPROM_SIZE))
            {
                if(offset >= 0)
                {
                    eeprom_lptr += (uint32_t)offset;
                }
                else
                {
                    offset = -offset;
                    eeprom_lptr -= (uint32_t)offset;
                }
                ret = DSUCCESS;
            }
            break;

        default:
            break;
        }
    }
    return ret;
}


/*******************************************************************************
 * Function       : EEPROM_ioctl
 * Author         : Linfei
 * Date           : 2012.08.11
 * Description    : EEPROM写保护控制
 * Calls          : None
 * Input          : op：写保护设置参数，取值范围：
 *                      EEPROM_PRO_NONE          不保护
 *                      EEPROM_PRO_UPER_QUARTER  保护高地址端1/4存储区
 *                      EEPROM_PRO_UPER_HALF     保护高地址端1/2存储区
 *                      EEPROM_PRO_ALL           保护所有存储区
 *                  arg：未用
 * Output         : None
 * Return         : DSUCCESS: 设置成功；OPFAULT：设置失败
 ********************************************************************************
 *-------------------------------------------------------------------------------
 * 2012.08.11 : 1.0.0 : Linfei
 * Modification   : 初始代码编写
 ********************************************************************************/
int32_t EEPROM_ioctl(uint32_t op, void *arg)
{
    int32_t ret = OPFAULT;
    arg = arg;
    if((eeprom_opened == true) && (IS_VALID_EEPROM_PROCODE(op)))
    {
        SPI_EEPROM_WriteEnable();
        SPI_EEPROM_CS_LOW;
        (void)SPI_EEPROM_SendByte(EEPROM_WRSR);
        (void)SPI_EEPROM_SendByte((uint8_t)op);
        SPI_EEPROM_CS_HIGH;
    
        /* 等待写结束 */
#ifdef EEPROM_WRITE_WAIT    
        eeprom_wait_count = EEPROM_WAIT_TIME;
        while(eeprom_wait_count != 0U)
        {
        }
#endif

        ret = DSUCCESS;

#ifdef EEPROM_WRITE_WAIT
        if(SPI_EEPROM_WaitForWriteEnd() == FALSE)
        {
            ret = OPFAULT;
        }
#endif
    }
    return ret;
}

/*******************************************************************************
 * Function       : EEPROM_close
 * Author         : Linfei
 * Date           : 2012.08.11
 * Description    : 关闭EEPROM，SPI控制器时钟复位
 * Calls          : SPI_I2S_DeInit
 * Input          : None
 * Output         : None
 * Return         : DSUCCESS：关闭成功 
 ********************************************************************************
 *-------------------------------------------------------------------------------
 * 2012.08.11 : 1.0.0 : Linfei
 * Modification   : 初始代码编写
 ********************************************************************************/
int32_t EEPROM_close(void)
{
    if (eeprom_opened == true)
    {
        SPI_I2S_DeInit(EEPROM_SPI_PORT);
        EEPROM_SPI_RCC_CLOSE;//wujing 2013.1.8 for low power control
        eeprom_opened = false;
    }
    return DSUCCESS;
}





