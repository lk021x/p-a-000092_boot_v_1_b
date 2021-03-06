/*****************************************************
          modbus-rtu 通讯规约
******************************************************/

#include "modbus.h"

extern uint8_t g_commaddr;

unsigned short getCRC16(volatile unsigned char *ptr,unsigned char len) ; 
//void Delaytime(vu32 nCount);
/***************************************
函数名称：crc16校验
函数功能：crc16校验
函数输入：字节指针*ptr，数据长度len
函数返回：双字节crc
****************************************/

unsigned short getCRC16(volatile unsigned char *ptr,unsigned char len) 
{ 
    unsigned char i; 
    unsigned short crc = 0xFFFF; 
    if(len==0) 
    {
        len = 1;
    } 
    while(len--)  
    {   
        crc ^= *ptr; 
        for(i=0; i<8; i++)  
        { 
            if(crc&1) 
            { 
                crc >>= 1;  
                crc ^= 0xA001; 
            }  
            else 
            {
                crc >>= 1;
            } 
        }         
        ptr++; 
    } 
    return(crc); 
} 
/***************************************
功能：读写地址的有效性判断,第一个参数是首地址，第二个参数是读写的长度，第三个参数是指MODBUS命令
****************************************/
/*u8 Modbus_Addr(uint16_t ubeginaddr,uint8_t ulen,uint8_t ucmd)
{
    //地址有效返回1，无效返回0 ；

    if(ucmd == 0x03)
    {
        if(ubeginaddr > 4998 && ubeginaddr+ulen < 5047)  
        {
           return 1;
        } 
        else
        {
           return 0 ;
        }  
    }
    
    if(ucmd == 0x04) 
    {
        if(ubeginaddr > 4998 && ubeginaddr+ulen < 5024)  
        {
           return 1;
        } 
        else
        {
           return 0 ;
        }  
    }
    if(ucmd == 0x06 || ucmd == 0x10) 
    {
        if(ubeginaddr > 4998 && ubeginaddr+ulen < 5047)  
        {
           return 1;
        } 
        else
        {
           return 0 ;
        }  
    }
    
    return 0 ;

}*/
/***************************************
功能：03命令处理函数，03在MODBUS里面是读N个寄存器的值
****************************************/
/////////////////////////////////////////////////////////////////////// 
int Modbus_Function_3(uint8_t* pdata,int datalen,uint8_t *sendbuf) 
{ 
    uint16_t tempdress = 0;
    uint16_t crcresult;
    uint16_t sendlen = 0 ;
    tempdress = (pdata[2] << 8) + pdata[3]; //得到读取的启始地址
    
    //？？ 这里需要增加对访问地址是否越限的判断
    uint8_t tx_flat = 0;
    tx_flat = Modbus_Addr(tempdress,pdata[5],0x03);
   
    if(tx_flat == 1)
    {
        sendbuf[0] = pdata[0]; //站地址
        sendbuf[1] = 0x03;  //命令符号
        sendbuf[2] = 2 * pdata[5]; //请求字节数（4B*2）
    
        Modbus_Read_Data(0x03,tempdress,&sendbuf[3],sendbuf[2]);
    
        sendlen = 2 * pdata[5] + 3;
    }
    else 
    {            
         sendbuf[0]=pdata[0]; //站地址
         sendbuf[1]=0x03|0x80; 
         sendbuf[2]=0x02;
         sendlen = 3 ;
    }
    
    crcresult = getCRC16(sendbuf,sendlen); 
    sendbuf[sendlen] = crcresult & 0xff; 
    sendbuf[sendlen+1] = (crcresult >> 8) & 0xff;
    sendlen = sendlen+2;
    return sendlen;
} 
//////////////////////////////////////////////
/***************************************
功能：04命令处理函数，04在MODBUS里面是读4X地址的数据
****************************************/
//////////////////////////////////////////////
int Modbus_Function_4(uint8_t* pdata,int datalen,uint8_t *sendbuf) 
{
    uint16_t tempdress = 0;
    uint16_t crcresult;
    uint16_t sendlen = 0 ;           
    tempdress = (pdata[2] << 8) + pdata[3]; //得到读取的启始地址
    
    uint16_t tx_flat = Modbus_Addr(tempdress,pdata[5],0x04);
    if(tx_flat == 0x01)
    {
        sendbuf[0] = pdata[0];//站地址
        sendbuf[1] = 0x04;  //命令符号
        sendbuf[2] = 2 * pdata[5]; //请求字节数（4B*2）
        sendlen = 2 * pdata[5] + 3;

        Modbus_Read_Data(0x04,tempdress,&sendbuf[3],sendbuf[2]);
    }
    else 
    {            
         sendbuf[0]=pdata[0]; //站地址
         sendbuf[1]=0x04|0x80; 
         sendbuf[2]=0x02;
    
         sendlen = 3 ;
    }

    crcresult = getCRC16(sendbuf,sendlen); 
    sendbuf[sendlen] = crcresult & 0xff; 
    sendbuf[sendlen+1] = (crcresult >> 8) & 0xff; 
    sendlen = sendlen+2;
    return sendlen;    
}
//////////////////////////////////////////////
/***************************************
功能：06命令处理函数，06在MODBUS里面是写1个寄存器的值
****************************************/
//////////////////////////////////////////////
int Modbus_Function_6(uint8_t* pdata,int datalen,uint8_t *sendbuf) 
{ 
    uint16_t tempdress = 0;
    uint8_t tx_flat = 1;
    uint16_t crcresult;
    uint16_t      sendlen = 0 ;
    tempdress = (pdata[2]<<8) + pdata[3];
    
    tx_flat = Modbus_Addr(tempdress,1,0x06);
    
    
    //如果是有效的地址，则设置发送标志
    //注意：如果是返回错误的话，需要修改下面的代码，当前的处理是如果是无效的地址，则不返回

    if(tx_flat == 1)
    {
        // ????设置值
        if (Modbus_Write_Data(tempdress,&pdata[4],2)>0){
            sendbuf[0] = pdata[0]; //站地址
            sendbuf[1] = 0x06; 
            sendbuf[2] = pdata[2]; 
            sendbuf[3] = pdata[3]; 
            sendbuf[4] = pdata[4]; 
            sendbuf[5] = pdata[5]; 
            sendlen = 6;
        }else{
            sendbuf[0] = pdata[0]; //站地址
            sendbuf[1] = 0x06|0x80; 
            sendbuf[2] = 0x02; 
         
            sendlen = 3;
        }
         
    }
    else
    {
        sendbuf[0] = pdata[0]; //站地址
        sendbuf[1] = 0x06|0x80; 
        sendbuf[2] = 0x02; 
     
        sendlen = 3;
    }
    crcresult = getCRC16(sendbuf,sendlen); 
    sendbuf[sendlen] = crcresult & 0xff; 
    sendbuf[sendlen+1] = (crcresult >> 8) & 0xff; 
    sendlen = sendlen+2; 
    return sendlen;
} 

//////////////////////////////////////////////
/***************************************
功能：10命令处理函数，06在MODBUS里面是写N个寄存器的值
****************************************/
//////////////////////////////////////////////
int Modbus_Function_10(uint8_t* pdata,int datalen,uint8_t *sendbuf) 
{ 
    uint16_t tempdress = 0;
    uint8_t tx_flat = 1;
    uint16_t crcresult;
    uint16_t      sendlen = 0 ;  
    tempdress = (pdata[2]<<8) + pdata[3];
    
    tx_flat = Modbus_Addr(tempdress,pdata[5],0x10);
  
    if(tx_flat == 1)
    {
        if (Modbus_Write_Data(tempdress,&pdata[7],pdata[6])>0){
            sendbuf[0] = pdata[0];
            sendbuf[1] = 0x10; 
            sendbuf[2] = pdata[2]; 
            sendbuf[3] = pdata[3]; 
            sendbuf[4] = pdata[4]; 
            sendbuf[5] = pdata[5]; 
            sendlen = 6;
        }else{
            sendbuf[0] = pdata[0]; //站地址
            sendbuf[1] = 0x10|0x80; 
            sendbuf[2] = 0x02; 
         
            sendlen = 3;
        }
    }
    else
    {
        sendbuf[0] = pdata[0]; //站地址
        sendbuf[1] = 0x10|0x80; 
        sendbuf[2] = 0x02; 
     
        sendlen = 3;
    }
    crcresult = getCRC16(sendbuf,sendlen); 
    sendbuf[sendlen] = crcresult & 0xff; 
    sendbuf[sendlen+1] = (crcresult >> 8) & 0xff; 
    sendlen = sendlen+2; 
    return sendlen;
} 

int Modbus_Function_error(uint8_t* pdata,int datalen,uint8_t *sendbuf) 
{
     uint16_t crcresult;
     uint16_t sendlen = 0 ;  
     sendbuf[0] = pdata[0]; //站地址
     sendbuf[1] = pdata[1]|0x80; 
     sendbuf[2] = 0x01; 
             
     sendlen = 3;
        
     crcresult = getCRC16(pdata,sendlen); 
     sendbuf[sendlen] = crcresult & 0xff; 
     sendbuf[sendlen+1] = (crcresult >> 8) & 0xff; 
     sendlen = sendlen+2; 
     return sendlen;
}
///////////////////////////////////////////////////////////// 
int ModbusDataDeal(uint8_t* pdata,int datalen,uint8_t *sendbuf) 
{ 
    int ret=0;

    if(pdata[0] == g_commaddr)//地址错误不应答
    {
        uint16_t crcresult;
        uint8_t tmp[2]={0};
        crcresult = getCRC16(pdata,datalen-2);
        tmp[1] = crcresult & 0xff; 
        tmp[0] = (crcresult >> 8) & 0xff;
        if((pdata[datalen-1] == tmp[0])&&(pdata[datalen-2] == tmp[1]))//crc校验错误不应答 
        { 
            switch(pdata[1])
            { 
            case 0x03:
                ret=Modbus_Function_3(pdata,datalen,sendbuf);
                break;  
            case 0x06:
                ret=Modbus_Function_6(pdata,datalen,sendbuf);
                break;
            case 0x10:
                ret=Modbus_Function_10(pdata,datalen,sendbuf); 
                break; 
            case 0x04:
                ret=Modbus_Function_4(pdata,datalen,sendbuf);
                break;    
            default:
                ret=Modbus_Function_error(pdata,datalen,sendbuf);
                break;
            }
        }
        else
        {
            ret=Modbus_Function_error(pdata,datalen,sendbuf); 
        }
    }
    return ret;
} 
/*******************************************************************************
* Function Name  : Delay
* Description    : Inserts a delay time.
* Input          : nCount: specifies the delay time length.
* Output         : None
* Return         : None
*******************************************************************************/
/*void Delaytime(vu32 nCount)
{
  for(; nCount != 0; nCount--);
}*/

/***************************************
功能：读N个字节数据，
第一个参数为所读的地址类型 3X地址=0X03   4X地址=0X04
第二个参数为读取的首地址
第三个参数为读取数据保存的缓冲区
第三个参数为读取的字节数
****************************************/
/*void Modbus_Read_Data(u16 utype,u16 uaddr,u8* pbuf,u8 ulen)
{


}*/

/***************************************
功能：写N个字节数据
第一个参数为写入的首地址
第二个参数为写入的缓冲区
第三个参数为写入的字节数
****************************************/
/*void Modbus_Write_Data(u16 uaddr,u8* pbuf,u8 ulen)
{
}*/
