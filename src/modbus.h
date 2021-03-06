#ifndef _MODBUS_H_
#define _MODBUS_H_ 1

#include "utilities.h"

//MODBUS 命令码
#define MODBUS_CMD_02  0x02 //读离散量输入
#define MODBUS_CMD_03  0x03 //读保持寄存器
#define MODBUS_CMD_04  0x04 //读输入寄存器
#define MODBUS_CMD_05  0x05 //写单个线圈
#define MODBUS_CMD_06  0x06 //写单个寄存器
#define MODBUS_CMD_15  0x0F //写多个线圈
#define MODBUS_CMD_16  0x10 //写多个寄存器
#define MODBUS_CMD_20  0x14 //读文件记录
#define MODBUS_CMD_21  0x15 //写文件记录
#define MODBUS_CMD_22  0x16 //屏蔽写寄存器
#define MODBUS_CMD_23  0x17 //读/写多个寄存器
#define MODBUS_CMD_43  0x2B //读设备识别码

//MODBUS 异常码
#define MODBUS_UNUSED  0x01 //非法功能
#define MODBUS_ABADDR  0x02 //非法数据地址
#define MODBUS_DATAER  0x03 //非法数据值
#define MODBUS_DEVICE  0x04 //从站设备故障
#define MODBUS_SURE    0x05 //确认
#define MODBUS_BUSY    0x06 //从属设备忙
#define MODBUS_FLASH   0x07 //存储奇偶性差错

//具体命令函数
int Modbus_Function_3(uint8_t* pdata,int datalen,uint8_t *sendbuf); 
int Modbus_Function_4(uint8_t* pdata,int datalen,uint8_t *sendbuf); 
int Modbus_Function_6(uint8_t* pdata,int datalen,uint8_t *sendbuf); 
int Modbus_Function_10(uint8_t* pdata,int datalen,uint8_t *sendbuf); 
int Modbus_Function_error(uint8_t* pdata,int datalen,uint8_t *sendbuf);


//在接收到一个完整数据包后调用，第一个参数是串口句柄，第二个是接收缓冲取，第三个是接收长度
int ModbusDataDeal(uint8_t*,int,uint8_t *sendbuf); 

//以下函数需要重写


/***************************************
功能：读写地址的有效性判断,第一个参数是首地址，第二个参数是读写的长度，第三个参数是指MODBUS命令(03\04\06\10)
地址有效返回1，无效返回0
****************************************/
uint8_t Modbus_Addr(uint16_t,uint8_t,uint8_t);

/***************************************
功能：读N个字节数据，
第一个参数为所读的地址类型 3X地址=0X03   4X地址=0X04
第二个参数为读取的首地址
第三个参数为读取数据保存的缓冲区
第三个参数为读取的字节数
****************************************/
void Modbus_Read_Data(uint16_t,uint16_t,uint8_t*,uint8_t);


/***************************************
功能：写N个字节数据
第一个参数为写入的首地址
第二个参数为写入的缓冲区
第三个参数为写入的字节数
****************************************/
int Modbus_Write_Data(uint16_t,uint8_t*,uint8_t);

#endif
