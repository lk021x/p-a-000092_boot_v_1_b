//------------------------------------------------------------------------------
// Protability:       c99.
// Design Pattern:    None.
// Base Classes:      None.
// MultiThread Safe:  None.
// Exception Safe:    None.
// Library/package:   stm32f10x.lib.
// Source files:      main.c.
// Related Document:  None.
// Organize:          sungrow copyright 2010
// Email:             llemmx@gmail.com
//------------------------------------------------------------------------------
// Release Note:
// file info
//------------------------------------------------------------------------------
// Version    Date          Author    Note
//------------------------------------------------------------------------------
// 1.0.0      2010-07-13    llemmx    None
// 1.0.16     2010-08-25    llemmx    None
//------------------------------------------------------------------------------
/* Includes ------------------------------------------------------------------*/
//HAL
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_flash.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_crc.h"
#include "misc.h"
#include "core_cm4.h"
#include "device.h"
#include "modbus.h"
#include "fcntl.h"
/* Exported types ------------------------------------------------------------*/
typedef  void (*pFunction)(void);
//GPIO端口定义
typedef struct
{
    GPIO_TypeDef *Base;//端口基地址
    u16 pin;           //端口位置
    GPIOMode_TypeDef  dir;//方向
} TGPIO;
// /* Exported constants --------------------------------------------------------*/
// #define NULL  0
// #define BOOL  unsigned char
// #define TRUE  1
// #define FALSE 0

//flash相关宏
//#define BLOCK_SIZE      127 //块尺寸

#if defined(STM32F10X_HD)
#define IAP_PAGE_SIZE       2048 //页大小

#elif defined(STM32F10X_CL)
#define IAP_PAGE_SIZE       2048 //页大小

#elif defined(STM32F10X_MD)
#define IAP_PAGE_SIZE       1024 //页大小

#else
#define IAP_PAGE_SIZE       2048
#endif



//升级状态
#define IAP_SREADY     0x0001 //准备中
#define IAP_SUPSYS     0x0002 //正在更新系统
#define IAP_SUPDATING  0x0004 //正在升级
#define IAP_SUPFAILED  0x0008 //升级失败
#define IAP_SFWRFAILED 0x0010 //flash读写失败
#define IAP_SUPSUCCESS 0x0020 //升级成功
#define IAP_SNOUPDATE  0x0040 //没有升级
#define IAP_SDATACHKF  0x0080 //数据效验失败
#define IAP_SBOOTUP    0x0100 //boot区升级
#define IAP_SFRPROTECT 0x0200 //flash读保护


/* V2升级状态 */
#define IAPV2_NO_UPDATE      0x0001 /* 未升级 */
#define IAPV2_UPDATING       0x0002 /* 正在升级 */
#define IAPV2_FAULT          0x0003 /* 故障 */
#define IAPV2_SUCCESSED      0x0004 /* 升级成功 */
#define IAPV2_SUBUPDATING    0x0005 /* 正在向下级升级 */
#define IAPV2_SUBSUCCESSED   0x0006 /* 向下升级结束 */

#define IAP_BASE                0x6000
#define IAP_FLASH_BASE          0x700000
#define IAP_FLASH_BACKUP_BASE   0x600000

/* Exported macro ------------------------------------------------------------*/
#define ApplicationAddress    0x08004000

#define MCU_FALSH_START_ADDR  0x08000000    // MCU的FLASH起始地址
#define MCU_FLASH_SIZE        0x100000      // 1M

#define PRODUCT_TYPE_ADDRESS                0x01  // 产品类型
#define PC_COMADDR_ADDRESS                  0x05   // 通信地址


#define IAP_FLASH_MAX_SIZE    (MCU_FLASH_SIZE - (ApplicationAddress - MCU_FALSH_START_ADDR))
/* Exported functions ------------------------------------------------------- */

/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbyte */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbyte */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbyte */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbyte */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbyte */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbyte */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbyte */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbyte */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbyte */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbyte */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbyte */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbyte */


/* Global variant ----------------------------------------------------------- */
pFunction Jump_To_Application;
uint32_t       g_timetick = 0;
uint8_t        g_buf[2048];//flash写缓冲
uint8_t        crcbuf[IAP_PAGE_SIZE];
uint8_t        g_comrbuf[256];//串口接收缓冲
uint8_t        g_comsbuf[256];//串口发送缓冲
//TFlashSave     g_flash={.block=0,.blocksize=0,.size=0};
uint8_t        g_commaddr;//通讯地址
uint8_t        product_type = 0;      //设备类型 //lixia;2011.8.23

uint8_t  iap_downover = 0; //下载结束标志
uint16_t iap_status = 0x0100; //iap状态
uint16_t iap_cmd = 0;      //升级指令
uint32_t iap_len_sum = 0;   //包长
uint32_t iap_getlen = 0;   //已经保存长度
uint32_t iap_crc32 = 0;    //数据包CRC32值
uint32_t iap_ver = 0;
uint16_t iap_blocksize = 0; //块尺寸
uint16_t iap_block = 0;    //块数量
uint16_t iap_upstat;       //升级状况

uint32_t iap_backup_crc32 = 0; /* 备份区CRC */


uint16_t iapv2_status;

uint32_t g_delay;

int fSystick;
int fSerial;
int fFM3104;
int fM25PE20;
int fextwdg;
uint8_t m_EXWDG;

void iapv2_erase_exterflash(uint32_t exteraddr, uint32_t len);

//用户程序跳转
void JumpUserApplication(void)
{
    uint32_t JumpAddress = 0;
    JumpAddress = *(__IO uint32_t *) (ApplicationAddress + 4);
    // JumpAddress = 0x8004000;
    /* Jump to user application */
    Jump_To_Application = (pFunction) JumpAddress;
    /* Initialize user application's Stack Pointer */
    __set_MSP(*(__IO uint32_t *) ApplicationAddress);
    Jump_To_Application();
}

/**
  * @brief  Gets the sector of a given address
  * @param  Address: Flash address
  * @retval The sector of a given address
  */
static uint32_t GetSector(uint32_t Address)
{
    uint32_t sector = 0;

    if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
    {
        sector = FLASH_Sector_0;
    }
    else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
    {
        sector = FLASH_Sector_1;
    }
    else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
    {
        sector = FLASH_Sector_2;
    }
    else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
    {
        sector = FLASH_Sector_3;
    }
    else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
    {
        sector = FLASH_Sector_4;
    }
    else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
    {
        sector = FLASH_Sector_5;
    }
    else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
    {
        sector = FLASH_Sector_6;
    }
    else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
    {
        sector = FLASH_Sector_7;
    }
    else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
    {
        sector = FLASH_Sector_8;
    }
    else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
    {
        sector = FLASH_Sector_9;
    }
    else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
    {
        sector = FLASH_Sector_10;
    }
    else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
    {
        sector = FLASH_Sector_11;
    }
    return sector;
}


void FLASH_DisableWriteProtectionPages(void)
{
    __IO uint32_t UserStartSector = FLASH_Sector_1, UserWrpSectors = OB_WRP_Sector_1;

    /* Get the sector where start the user flash area */
    UserStartSector = GetSector(ApplicationAddress);

    /* Mark all sectors inside the user flash area as non protected */
    UserWrpSectors = 0xFFF - ((1 << (UserStartSector / 8)) - 1);

    /* Unlock the Option Bytes */
    FLASH_OB_Unlock();

    /* Disable the write protection for all sectors inside the user flash area */
    FLASH_OB_WRPConfig(UserWrpSectors, DISABLE);

    /* Start the Option Bytes programming process. */
    if (FLASH_OB_Launch() != FLASH_COMPLETE)
    {
        /* Error: Flash write unprotection failed */
        //return (2);
    }
}

uint8_t WritePage()
{
    uint32_t FlashDestination = ApplicationAddress;
    FLASH_Status FLASHStatus = FLASH_COMPLETE;

    FlashDestination += iap_block * IAP_PAGE_SIZE;
    for (uint32_t i = 0; i < IAP_PAGE_SIZE; i = i + 4)
    {
        /* Program the data received into STM32F10x Flash */
        FLASHStatus = FLASH_ProgramWord(FlashDestination, *(uint32_t *)(g_buf + i));
        if (FLASHStatus != FLASH_COMPLETE)
            return 0;
        FlashDestination += 4;
    }
    return 1;
}

FLASH_Status erase_page(uint32_t addr)
{
    FLASH_Status FLASHStatus = FLASH_COMPLETE;
    if((addr == ADDR_FLASH_SECTOR_0) || \
            (addr == ADDR_FLASH_SECTOR_1) || \
            (addr == ADDR_FLASH_SECTOR_2) || \
            (addr == ADDR_FLASH_SECTOR_3) || \
            (addr == ADDR_FLASH_SECTOR_4) || \
            (addr == ADDR_FLASH_SECTOR_5) || \
            (addr == ADDR_FLASH_SECTOR_6) || \
            (addr == ADDR_FLASH_SECTOR_7) || \
            (addr == ADDR_FLASH_SECTOR_8) || \
            (addr == ADDR_FLASH_SECTOR_9) || \
            (addr == ADDR_FLASH_SECTOR_10) || \
            (addr == ADDR_FLASH_SECTOR_11))
    {
        FLASHStatus = FLASH_EraseSector(GetSector(addr), VoltageRange_3);
    }
    return FLASHStatus;
}


uint8_t program_page(void)
{
    FLASH_Status FLASHStatus = FLASH_COMPLETE;
    uint32_t utmp;

    __disable_irq();
    /* Unlock the Flash Bank1 Program Erase controller */
    FLASH_Unlock();
    /* Clear All pending flags */
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | \
                    FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | \
                    FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    //擦除1页
    utmp = ApplicationAddress + iap_block * IAP_PAGE_SIZE;
    if (utmp < ApplicationAddress)
    {
        return 0;
    }


    FLASHStatus = erase_page(utmp);


    if (FLASH_COMPLETE == FLASHStatus)
    {
        if (!WritePage())
            return 0;
        lmemset(g_buf, 0xFF, IAP_PAGE_SIZE);
    }
    FLASH_Lock();
    iap_block++;
    __enable_irq();

    return 1;
}

uint8_t save_program(uint8_t *buf, uint8_t size)
{
    uint16_t len;
//    uint32_t tmp;

    if ((iap_blocksize + size) > IAP_PAGE_SIZE)
    {
        len = IAP_PAGE_SIZE - iap_blocksize;
        lmemcpy(g_buf + iap_blocksize, buf, len);
        size -= len;
        buf += len;
        iap_blocksize = 0;
        if (!program_page())
            return 0;
        //保存长度
//        lseek(fFM3104, FM_EEPROM_BASE + IAP_BASE + 16, SEEK_SET);
//        tmp = iap_block * IAP_PAGE_SIZE;
//        write(fFM3104, (char *)&tmp, 4);
    }
    //保存数据到缓存
    lmemcpy(g_buf + iap_blocksize, buf, size);
    iap_blocksize += size;
    if (IAP_PAGE_SIZE <= iap_blocksize)
    {
        iap_blocksize = 0;
        if (!program_page())
            return 0;
//        lseek(fFM3104, FM_EEPROM_BASE + IAP_BASE + 16, SEEK_SET);
//        tmp = iap_block * IAP_PAGE_SIZE;
//        write(fFM3104, (char *)&tmp, 4);
    }
    return 1;
}

// modbus 协议 ----------------------------------------------------------------
//modbus命令判断
uint8_t Modbus_Addr(uint16_t ubeginaddr, uint8_t ulen, uint8_t ucmd)
{
    //地址有效返回1，无效返回0
    if(ucmd == MODBUS_CMD_04)
    {
        if(ubeginaddr > 19998 && ubeginaddr + ulen < 20004)
        {
            return 1;
        }
        else if (ubeginaddr > 20008 && ubeginaddr + ulen < 20019)
        {
            return 1;
        }
        else if (ubeginaddr == 4999)  //lixia;2011.8.23;增加读设备类型编码
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    if(ucmd == MODBUS_CMD_06 || ucmd == MODBUS_CMD_16)
    {
        if(ubeginaddr >= 20003 && ubeginaddr + ulen < 21021)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

void Modbus_Read_Data(uint16_t cmd, uint16_t addr, uint8_t *sbuf, uint8_t len)
{
    uint16_t curaddr = addr;
    uint16_t index = 0;
    if (MODBUS_CMD_04 == cmd)
    {
        if(curaddr == 4999)
        {
            sbuf[index++] = 0;             //lixia;2011.8.23;传设备类型编码
            sbuf[index++] = product_type     & 0xFF;
        }
        if (index < len)
            curaddr = (index >> 1) + addr;
        else
            return ;

        if (curaddr >= 19999 && curaddr <= 20000)
        {
            sbuf[index++] = 0x00;
            sbuf[index++] = 0x01;
            sbuf[index++] = 0x00;
            sbuf[index++] = 0x00;
        }
        if (index < len)
            curaddr = (index >> 1) + addr;
        else
            return ;
        if (curaddr == 20001)
        {
            sbuf[index++] = (iap_status >> 8) & 0xFF;
            sbuf[index++] = iap_status     & 0xFF;
        }
        if (index < len)
            curaddr = (index >> 1) + addr;
        else
            return ;
        if (curaddr == 20002)
        {
            sbuf[index++] = 0x00;
            sbuf[index++] = 0x0F;
        }
        if (index < len)
            curaddr = (index >> 1) + addr;
        else
            return ;
        if (curaddr == 20008)
        {
            sbuf[index++] = 0; //(iap_level>> 8) & 0xFF;
            sbuf[index++] = 0; //(iap_level    ) & 0xFF;
        }
        if (index < len)
            curaddr = (index >> 1) + addr;
        else
            return ;
        if (curaddr == 20009)
        {
            sbuf[index++] = 0; //(iap_addr>> 8) & 0xFF;
            sbuf[index++] = 0; //(iap_addr    ) & 0xFF;
        }
        if (index < len)
            curaddr = (index >> 1) + addr;
        else
            return ;
        if (curaddr >= 20010 && curaddr < 20011)
        {
            sbuf[index++] = (iap_crc32 >> 24) & 0xFF;
            sbuf[index++] = (iap_crc32 >> 16) & 0xFF;
            sbuf[index++] = (iap_crc32 >> 8) & 0xFF;
            sbuf[index++] = (iap_crc32    ) & 0xFF;
        }
        if (index < len)
            curaddr = (index >> 1) + addr;
        else
            return ;
        if (curaddr >= 20012 && curaddr < 20013)
        {
            sbuf[index++] = (iap_ver >> 24) & 0xFF;
            sbuf[index++] = (iap_ver >> 16) & 0xFF;
            sbuf[index++] = (iap_ver >> 8) & 0xFF;
            sbuf[index++] = (iap_ver    ) & 0xFF;
        }
        if (index < len)
            curaddr = (index >> 1) + addr;
        else
            return ;
        if (curaddr >= 20014 && curaddr < 20015)
        {
            sbuf[index++] = (iap_len_sum >> 24) & 0xFF;
            sbuf[index++] = (iap_len_sum >> 16) & 0xFF;
            sbuf[index++] = (iap_len_sum >> 8) & 0xFF;
            sbuf[index++] = (iap_len_sum    ) & 0xFF;
        }
        if (index < len)
            curaddr = (index >> 1) + addr;
        else
            return ;
        if (curaddr >= 20016 && curaddr < 20017)
        {
            sbuf[index++] = (iap_getlen >> 24) & 0xFF;
            sbuf[index++] = (iap_getlen >> 16) & 0xFF;
            sbuf[index++] = (iap_getlen >> 8) & 0xFF;
            sbuf[index++] = (iap_getlen    ) & 0xFF;
        }
    }
}

//eeprom 存储划分
//|-起始地址-|-结束地址-|-长度-|-说明
//|-0       -|-1       -|-2B  -|-升级命令(0-没有升级,2-Boot升级,3-IAP升级)
//|-2       -|-3       -|-2B  -|-升级状态(0-没有升级,0x20-升级完成)
//|-4       -|-7       -|-4B  -|-升级包长度
//|-8       -|-11      -|-4B  -|-数据包CRC32
//|-12      -|-15      -|-4B  -|-升级包版本
//|-16      -|-19      -|-4B  -|-已经升级长度
int Modbus_Write_Data(uint16_t addr, uint8_t *sbuf, uint8_t len)
{
    uint16_t curaddr = addr;
    uint16_t index = 0;

    if (curaddr >= 20003 && curaddr <= 20007)
    {
#if 0
        uint8_t  cmd;
        cmd = sbuf[index+1] & 0xFF;
        if ((sbuf[index] >> 7) & 0x01) //不支持加密
        {
            return -1;
        }
        if((cmd == 2) || (cmd == 3) || (cmd == 4)) //Boot升级
        {
        }
        else
        {
            return -1;
        }
#endif
        if (sbuf[index+2] != 0xAA || sbuf[index+3] != 0x55 || sbuf[index+4] != 0xAA || sbuf[index+5] != 0x55)
            return -1;
        if (sbuf[index+6] != 0x55 || sbuf[index+7] != 0xAA || sbuf[index+8] != 0x55 || sbuf[index+9] != 0xAA)
            return -1;
        //握手成功
        iap_status = IAP_SUPDATING;
        index += 10;
    }
    if (index < len)
        curaddr = (index >> 1) + addr;
    else
        return 1;
    if (curaddr == 20008)
    {
        //gGp.iap_level=(sbuf[index]<<8) | sbuf[index+1];
        index += 2;
    }
    if (index < len)
        curaddr = (index >> 1) + addr;
    else
        return 1;
    if (curaddr == 20009)
    {
        //gGp.iap_addr=(sbuf[index]<<8) | sbuf[index+1];
        index += 2;
    }
    if (index < len)
        curaddr = (index >> 1) + addr;
    else
        return 1;
    if (iap_status != IAP_SUPDATING)
        return -1;
    if (curaddr >= 20010 && curaddr <= 20011)
    {
        iap_crc32 = (sbuf[index] << 24) | (sbuf[index+1] << 16);
        iap_crc32 |= (sbuf[index+2] << 8) | sbuf[index+3];
        index += 4;
//        lseek(fFM3104, FM_EEPROM_BASE + IAP_BASE + 8, SEEK_SET);
//        write(fFM3104, (char *)&iap_crc32, 4);
    }
    if (index < len)
        curaddr = (index >> 1) + addr;
    else
        return 1;
    if (curaddr >= 20012 && curaddr <= 20013)
    {
        iap_ver = (sbuf[index] << 24) | (sbuf[index+1] << 16);
        iap_ver |= (sbuf[index+2] << 8) | sbuf[index+3];
        index += 4;
    }
    if (index < len)
        curaddr = (index >> 1) + addr;
    else
        return 1;
    if (curaddr >= 20014 && curaddr <= 20015)
    {
        iap_len_sum = (sbuf[index] << 24) | (sbuf[index+1] << 16);
        iap_len_sum |= (sbuf[index+2] << 8) | sbuf[index+3];
        index += 4;
        //保存到eeprom
//        lseek(fFM3104, FM_EEPROM_BASE + IAP_BASE + 4, SEEK_SET);
//        write(fFM3104, (char *)&iap_len_sum, 4);
        //DBPRINTF("iap_len_sum=%d\n",gGp.iap_len_sum);
    }
    if (index < len)
        curaddr = (index >> 1) + addr;
    else
        return 1;
    if (curaddr >= 20016 && curaddr <= 20017)
    {
        uint32_t utmp, utaddr;
        utmp = (sbuf[index] << 24) | (sbuf[index+1] << 16);
        utmp |= (sbuf[index+2] << 8) | sbuf[index+3];
        //恢复数据
        if (utmp < iap_getlen && utmp > 0)
        {
            iap_blocksize = utmp % IAP_PAGE_SIZE;
            if ((utmp / IAP_PAGE_SIZE) < iap_block)
            {
                iap_block = utmp / IAP_PAGE_SIZE;
                if (iap_blocksize > 0)
                {
                    utaddr = ApplicationAddress;
                    utaddr += (iap_block + 1) * IAP_PAGE_SIZE;
                    lmemcpy(g_buf, (uint8_t *)utaddr, IAP_PAGE_SIZE);
                }
            }
        }
        else if (utmp == 0)
        {
            iap_blocksize = 0;
            iap_block = 0;
        }
        iap_getlen = utmp;
        index += 4;
    }
    if (index < len)
        curaddr = (index >> 1) + addr;
    else
        return 1;
    if (curaddr >= 20018 && curaddr <= 20140)
    {
        uint16_t i = (sbuf[index] << 8) | sbuf[index+1];
        save_program(sbuf + index + 2, i & 0xFF);

        iap_getlen += i;
        if (iap_getlen >= iap_len_sum)
        {
            iap_downover = 1;
        }
    }
    return 1;
}



int lmemcmp(uint8_t *src1, uint8_t *src2, uint8_t size)
{
    while(size > 0 && *src1 == *src2)
    {
        ++src1;
        ++src2;
        --size;
    }
    return size;
}

void modbus_read_data()
{
    uint8_t buf[6];
    uint8_t zerobuf[64] = { 0 };
    lseek(fFM3104, FM_EEPROM_BASE + IAP_BASE, SEEK_SET);
    read(fFM3104, (char *)buf, 6); //特征字符
    read(fFM3104, (char *)&iap_cmd, 2); //读取当前命令
    read(fFM3104, (char *)&iap_upstat, 2); //读取当前升级状态
    read(fFM3104, (char *)&iap_backup_crc32, 4); // 获取备份区CRC
    lseek(fFM3104, FM_EEPROM_BASE + IAP_BASE + 46, SEEK_SET);
    read(fFM3104, (char *)&iap_crc32, 4); //CRC32值
    read(fFM3104, (char *)&iap_len_sum, 4); //读取总包长
    //read serial addr
    lseek(fFM3104, FM_EEPROM_BASE + PC_COMADDR_ADDRESS, SEEK_SET);
    read(fFM3104, &g_commaddr, 1); // 设备地址

    //特征字符串检查
    if(lmemcmp(buf, "SUNPWR", 6) != 0)
    {
        //写特征字符串
        iap_cmd   = 0;
        iap_upstat = IAPV2_NO_UPDATE; //设置为未升级
        iap_len_sum = 0;
        iap_crc32 = 0;
        lseek(fFM3104, FM_EEPROM_BASE + IAP_BASE + 6, SEEK_SET);
        write(fFM3104, (char *)&iap_cmd, 2);
        write(fFM3104, (char *)&iap_upstat, 2);
        write(fFM3104, (char *)&zerobuf, 52);
        
        lseek(fFM3104, FM_EEPROM_BASE + IAP_BASE, SEEK_SET);
        write(fFM3104, "SUNPWR", 6);
    }
}

/* 获取片内Flash程序CRC */
uint32_t get_interflash_crc(uint32_t addr, uint32_t len)
{
    uint32_t *dbuf = (uint32_t *)addr;
    uint32_t lcrc = 0;

    CRC_ResetDR();
    if ((len % 4) == 0)
        lcrc = CRC_CalcBlockCRC(dbuf, len / 4);
    else
        lcrc = CRC_CalcBlockCRC(dbuf, (len / 4) + 1);
    return lcrc;
}

/* 获取片外Flash程序CRC */
uint32_t get_exterflash_crc(uint32_t addr, uint32_t len)
{
    uint32_t i = 0, lcrc = 0, *ubuf;
    ubuf = (uint32_t *)crcbuf;
    CRC_ResetDR();
    lseek(fM25PE20, addr, SEEK_SET);
    for (i = 0; i < len / IAP_PAGE_SIZE; ++i)
    {
        read(fM25PE20, crcbuf, IAP_PAGE_SIZE);
        lcrc = CRC_CalcBlockCRC(ubuf, IAP_PAGE_SIZE >> 2);
		ioctl(fextwdg, FEED_EXTWDG, NULL);
    }
    //ioctl(fextwdg, FEED_EXTWDG, NULL);
    if ((len % IAP_PAGE_SIZE) > 0)
    {
        memset(crcbuf, 0, IAP_PAGE_SIZE);
        i = read(fM25PE20, crcbuf, len % IAP_PAGE_SIZE);
        if ((i % 4) == 0)
        {
            lcrc = CRC_CalcBlockCRC(ubuf, i / 4);
        }
        else
        {
            lcrc = CRC_CalcBlockCRC(ubuf, (i / 4) + 1);
        }
    }
    return lcrc;
}

/* 将升级程序从片外flash拷贝到片内flash */
void copy_code_to_interflash(uint32_t addr, uint32_t len)
{
    // 将升级程序从片外flash搬移至片内flash
    lseek(fM25PE20, addr, SEEK_SET);
    uint16_t i;
    iap_block = 0;
    iap_blocksize = 0;
    for (i = 0; i < len / IAP_PAGE_SIZE; ++i)
    {
        read(fM25PE20, (char *)g_buf, IAP_PAGE_SIZE);
        program_page();

        //喂狗
        ioctl(fextwdg, FEED_EXTWDG, NULL);
    }
    if ((len % IAP_PAGE_SIZE) > 0)
    {
        i = read(fM25PE20, (char *)g_buf, len % IAP_PAGE_SIZE);
        program_page();
        ioctl(fextwdg, FEED_EXTWDG, NULL);
    }
}

/* 将片内flash程序拷贝到片外flash，并返回 */
void copy_code_to_exterflash(uint32_t interaddr, uint32_t len, uint32_t exteraddr)
{
    uint32_t *dbuf = (uint32_t *)interaddr;
    iapv2_erase_exterflash(exteraddr, len);

    lseek(fM25PE20, exteraddr, SEEK_SET);
    uint16_t i;
    uint32_t index = 0;
    for (i = 0; i < len / IAP_PAGE_SIZE; ++i)
    {
        write(fM25PE20, (uint8_t *)&dbuf[index], IAP_PAGE_SIZE);
        index += IAP_PAGE_SIZE >> 2;
        //喂狗
        ioctl(fextwdg, FEED_EXTWDG, NULL);
    }
    if ((len % IAP_PAGE_SIZE) > 0)
    {
        write(fM25PE20, (uint8_t *)dbuf[index], len % IAP_PAGE_SIZE);
        ioctl(fextwdg, FEED_EXTWDG, NULL);
    }
}

/* 擦除片外flash */
void iapv2_erase_exterflash(uint32_t exteraddr, uint32_t len)
{
    W25QX_IOCTL_ARG iap_arg = {.addr = exteraddr, .dat = NULL};
    uint32_t opcode = 0;
    uint32_t step = 0;
    uint32_t endaddr = exteraddr + len;
    for (iap_arg.addr = exteraddr; iap_arg.addr < endaddr; iap_arg.addr += step)
    {
        if((iap_arg.addr + 0x10000) <= endaddr)
        {
            opcode = W25QX_CMD_BE64KB;
            step = 0x10000;
        }
        else if((iap_arg.addr + 0x8000) <= endaddr)
        {
            opcode = W25QX_CMD_BE32KB;
            step = 0x8000;
        }
        else
        {
            opcode = W25QX_CMD_SE;
            step = 0x1000;
        }
        ioctl(fM25PE20, opcode, &iap_arg);
        ioctl(fextwdg, FEED_EXTWDG, NULL);
    }
}

/* 从备份区恢复原有应用程序 */
bool restore_application(void)
{
    uint8_t try_times = 0;
    if(get_exterflash_crc(IAP_FLASH_BACKUP_BASE, IAP_FLASH_MAX_SIZE) == iap_backup_crc32)
    {
        try_times = 3;
        while(try_times > 0)
        {
            copy_code_to_interflash(IAP_FLASH_BACKUP_BASE, IAP_FLASH_MAX_SIZE);
            
            // 校验片内flash中的代码CRC
            if(get_interflash_crc(ApplicationAddress, IAP_FLASH_MAX_SIZE) == iap_backup_crc32)
            {
                break;
            }
            try_times--;
        }
    }
    if(try_times == 0)
    {
        return FALSE;
    }
    return TRUE;
}

/* 将应用程序备份到片外FLASH备份区 */
bool backup_application(void)
{
    uint8_t try_times = 0;
    iap_backup_crc32 = get_interflash_crc(ApplicationAddress, IAP_FLASH_MAX_SIZE);
    lseek(fFM3104, FM_EEPROM_BASE + IAP_BASE + 10, SEEK_SET);
    write(fFM3104, (char *)&iap_backup_crc32, 4);
    try_times = 3;
    while(try_times > 0)
    {
        copy_code_to_exterflash(ApplicationAddress, IAP_FLASH_MAX_SIZE, IAP_FLASH_BACKUP_BASE); // 5s
        if(get_exterflash_crc(IAP_FLASH_BACKUP_BASE, IAP_FLASH_MAX_SIZE) == iap_backup_crc32)   // 2.5s
        {
            break;
        }
        try_times--;
    }
    if(try_times == 0)
    {
        return FALSE;
    }
    return TRUE;
}




//-----------------------------------------------------------------------------
/*
  flash区域划分 ---------------------------------------------------------------
  地址0~4000是FRAM ------------------------------------------------------------
  |-0
*/
int main()
{
    uint8_t rbuf[256] = {0}; //串口接收的数据
    uint8_t sbuf[256] = {0}; //串口发送数据
    uint16_t ret = 0, slen = 0;
    TUsartX_ioctl uio;
    uint8_t try_times = 0;
    dev_init();

    fSystick = open("SYSTICK0", 0, 0);

    //复位稳定时间
    read(fSystick, &g_timetick, 4);
    g_delay = g_timetick;
    while((g_timetick - g_delay) < 500)
    {
        read(fSystick, &g_timetick, 4);
    }
    fFM3104 = open("FM31XX0", 0, 0);
    fM25PE20 = open("W25QX0", 0, 0);

    lseek(fFM3104, PRODUCT_TYPE_ADDRESS, SEEK_SET);
    read(fFM3104, (char *)&product_type, 1); //读取当前的设备类型//lixia;2011.8.23

    fextwdg = open("EXTWDG0", 0, 0);

    //喂狗
    ioctl(fextwdg, FEED_EXTWDG, NULL);

    modbus_read_data();

    ioctl(fextwdg, FEED_EXTWDG, NULL);
    
    if((iap_cmd == 3) && (iap_upstat == IAPV2_SUCCESSED)) /* 升级后的应用程序代码有问题，恢复原有程序 */
    {
        restore_application();
    }
    else if((iap_cmd == 3) && (iap_upstat == IAPV2_NO_UPDATE)) /* 外部缓存升级 */
    {
        // 校验片外flash中的代码CRC
        if(get_exterflash_crc(IAP_FLASH_BASE, iap_len_sum) == iap_crc32)  // 1.8s
        {
            /* 原有程序备份至片外FLASH备份区 */
            backup_application();  // 8.2s
            
            //关闭写保护
            FLASH_DisableWriteProtectionPages();
// #if DEBUG
//             W25QX_IOCTL_ARG arg = {.addr = 0x700000, .dat = NULL};
//             ioctl(fM25PE20, W25QX_CMD_BE32KB, &arg);
// #endif
            try_times = 3;
            while(try_times > 0)
            {
                copy_code_to_interflash(IAP_FLASH_BASE, iap_len_sum);
                
                // 校验片内flash中的代码CRC
                if(get_interflash_crc(ApplicationAddress, iap_len_sum) == iap_crc32)
                {
                    break;
                }
                try_times--;
            }
            
            if(try_times != 0)
            {
                lseek(fFM3104, FM_EEPROM_BASE + IAP_BASE + 8, SEEK_SET);
                iap_status = IAPV2_SUCCESSED;
                write(fFM3104, (char *)&iap_status, 2);
                ioctl(fextwdg, FEED_EXTWDG, NULL);
            }
            else
            {
                /* 代码拷贝失败，从片外备份区恢复原有程序 */
                restore_application();
            }
        }
        else
        {
            /* 校验失败，退出升级 */
            lseek(fFM3104, FM_EEPROM_BASE + IAP_BASE + 8, SEEK_SET);
            iap_status = IAPV2_NO_UPDATE;
            write(fFM3104, (char *)&iap_status, 2);
            ioctl(fextwdg, FEED_EXTWDG, NULL);
        }
    }
    goto OUT;

BOOTUP:

    uio.rbuf   = g_comrbuf;
    uio.rbuflen = 256;
    uio.sbuf   = g_comsbuf;
    uio.sbuflen = 256;

    fSerial = open("USART1", O_NONBLOCK, 0x0000); //打开串口
    ioctl(fSerial, USART_SET_BUF, &uio);
    ioctl(fSerial, USART_SET_485E, &uio);

    while(1)
    {
        read(fSystick, &g_delay, 4);
        read(fSystick, &g_timetick, 4);
        while(iap_downover == 0 && (g_timetick - g_delay) < 60000)
        {
            ret = read(fSerial, (char *)rbuf, 256);
            if (ret > 0)
            {
                slen = ModbusDataDeal(rbuf, ret, sbuf);
                if (slen > 0)
                {
                    write(fSerial, (char *)sbuf, slen);
                }
                read(fSystick, &g_delay, 4);
            }
            read(fSystick, &g_timetick, 4);
            //喂狗
            ioctl(fextwdg, FEED_EXTWDG, NULL);
        }
        if (g_timetick - g_delay > 59000) //超时
        {
            goto OUT;
        }
        //延时100ms
        read(fSystick, &g_timetick, 4);
        g_delay = g_timetick;
        while((g_timetick - g_delay) < 1000)
        {
            read(fSystick, &g_timetick, 4);
        }
        if (iap_downover)
        {
            program_page();
            //CRC32效验
            if(get_interflash_crc(ApplicationAddress, iap_len_sum) == iap_crc32)
            {
                lseek(fFM3104, FM_EEPROM_BASE + IAP_BASE + 6, SEEK_SET);
                iap_cmd = 0;
                write(fFM3104, (char *)&iap_cmd, 2);
                iap_status = IAPV2_SUCCESSED;
                write(fFM3104, (char *)&iap_status, 2);
                break;
            }
            else
            {
                iap_status = IAP_SDATACHKF;
                iap_block   = 0;
                iap_getlen  = 0;
                iap_downover = 0;
            }
            //喂狗
            ioctl(fextwdg, FEED_EXTWDG, NULL);
        }
    }
OUT:
    close(fSystick);
    close(fFM3104);
    close(fM25PE20);
    close(fextwdg);
    SysTick->CTRL = 0;
    JumpUserApplication();
}

