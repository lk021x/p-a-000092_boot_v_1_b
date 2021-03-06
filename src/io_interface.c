/********************************************************************************
 * Copyright (C), 1997-2011, SUNGROW POWER SUPPLY CO., LTD. 
 * File name      :io_interface.c
 * Author         :Xu Shun'an
 * Date           :2011.08.04
 * Description    :IO控制器的端口初始化
 * Others         :None
 *-------------------------------------------------------------------------------
 * 2011-08-04 : 1.0.0 : xusa
 * Modification   : 初始代码编写。
 ********************************************************************************/
#include "stm32f4xx_gpio.h"

// IO端口定义
typedef struct 
{
    GPIO_TypeDef *Base;     // 端口基地址
    GPIO_InitTypeDef io;    // IO口基本定义
    uint8_t       sinit;    // 端口初始化状态(0-无动作,1-低电平,2-高电平)
}TGPIOA;

TGPIOA const gGPIOAll[] = 
{
    {.Base = GPIOC, {.GPIO_Pin = GPIO_Pin_10, .GPIO_Mode = GPIO_Mode_AF, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},     // SPI3_SCK
    {.Base = GPIOC, {.GPIO_Pin = GPIO_Pin_11, .GPIO_Mode = GPIO_Mode_AF, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},     // SPI3_MISO
    {.Base = GPIOC, {.GPIO_Pin = GPIO_Pin_12, .GPIO_Mode = GPIO_Mode_AF, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},     // SPI3_MOSI
    {.Base = GPIOA, {.GPIO_Pin = GPIO_Pin_15, .GPIO_Mode = GPIO_Mode_OUT, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},    // SPI3_NSS


    {.Base = GPIOB, {.GPIO_Pin = GPIO_Pin_4, .GPIO_Mode = GPIO_Mode_OUT, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 1},     // 485_EN1
    {.Base = GPIOA, {.GPIO_Pin = GPIO_Pin_9, .GPIO_Mode = GPIO_Mode_AF, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},      // USART1_TX
    {.Base = GPIOA, {.GPIO_Pin = GPIO_Pin_10, .GPIO_Mode = GPIO_Mode_AF, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},     // USART1_RX
//     
//     {.Base = GPIOD, {.GPIO_Pin = GPIO_Pin_7, .GPIO_Mode = GPIO_Mode_OUT, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 1},     // 485_EN2
//     {.Base = GPIOD, {.GPIO_Pin = GPIO_Pin_5, .GPIO_Mode = GPIO_Mode_AF, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},      // UXART2_TX
//     {.Base = GPIOD, {.GPIO_Pin = GPIO_Pin_6, .GPIO_Mode = GPIO_Mode_AF, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},      // UXART2_RX
    
//     {.Base = GPIOD, {.GPIO_Pin = GPIO_Pin_11, .GPIO_Mode = GPIO_Mode_OUT, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 1},    // 485_EN3
//     {.Base = GPIOD, {.GPIO_Pin = GPIO_Pin_8, .GPIO_Mode = GPIO_Mode_AF, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},      // UART3_TX
//     {.Base = GPIOD, {.GPIO_Pin = GPIO_Pin_9, .GPIO_Mode = GPIO_Mode_AF, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},      // UART3_RX

//     {.Base = GPIOD, {.GPIO_Pin = GPIO_Pin_0, .GPIO_Mode = GPIO_Mode_AF, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},      // CAN1_RX
//     {.Base = GPIOD, {.GPIO_Pin = GPIO_Pin_1, .GPIO_Mode = GPIO_Mode_AF, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},      // CAN1_TX
//     
//     {.Base = GPIOB, {.GPIO_Pin = GPIO_Pin_5, .GPIO_Mode = GPIO_Mode_AF, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},      // CAN2_RX
//     {.Base = GPIOB, {.GPIO_Pin = GPIO_Pin_6, .GPIO_Mode = GPIO_Mode_AF, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},      // CAN2_TX
   
//     {.Base = GPIOB, {.GPIO_Pin = GPIO_Pin_0, .GPIO_Mode = GPIO_Mode_OUT, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 2},     //  NET_RESET
//     {.Base = GPIOA, {.GPIO_Pin = GPIO_Pin_2, .GPIO_Mode = GPIO_Mode_AF, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_NOPULL, .GPIO_Speed = GPIO_Speed_100MHz}, .sinit = 0}, //  ETH_RMII_MDIO
//     {.Base = GPIOC, {.GPIO_Pin = GPIO_Pin_1, .GPIO_Mode = GPIO_Mode_AF, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_NOPULL, .GPIO_Speed = GPIO_Speed_100MHz}, .sinit = 0}, //  ETH_RMII_MDC
//     {.Base = GPIOB, {.GPIO_Pin = GPIO_Pin_11, .GPIO_Mode = GPIO_Mode_AF, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_NOPULL, .GPIO_Speed = GPIO_Speed_100MHz}, .sinit = 0},//  ETH_RMII_TX_EN
//     {.Base = GPIOB, {.GPIO_Pin = GPIO_Pin_12, .GPIO_Mode = GPIO_Mode_AF, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_NOPULL, .GPIO_Speed = GPIO_Speed_100MHz}, .sinit = 0},//  ETH_RMII_TXD0
//     {.Base = GPIOB, {.GPIO_Pin = GPIO_Pin_13, .GPIO_Mode = GPIO_Mode_AF, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_NOPULL, .GPIO_Speed = GPIO_Speed_100MHz}, .sinit = 0},//  ETH_RMII_TXD1
//     {.Base = GPIOA, {.GPIO_Pin = GPIO_Pin_1, .GPIO_Mode = GPIO_Mode_AF, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_NOPULL, .GPIO_Speed = GPIO_Speed_100MHz}, .sinit = 0}, //  ETH_RMII_REF_CLK
//     {.Base = GPIOA, {.GPIO_Pin = GPIO_Pin_7, .GPIO_Mode = GPIO_Mode_AF, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_NOPULL, .GPIO_Speed = GPIO_Speed_100MHz}, .sinit = 0}, //  ETH_RMII_CRS_DV
//     {.Base = GPIOC, {.GPIO_Pin = GPIO_Pin_4, .GPIO_Mode = GPIO_Mode_AF, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_NOPULL, .GPIO_Speed = GPIO_Speed_100MHz}, .sinit = 0}, //  ETH_RMII_RXD0
//     {.Base = GPIOC, {.GPIO_Pin = GPIO_Pin_5, .GPIO_Mode = GPIO_Mode_AF, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_NOPULL, .GPIO_Speed = GPIO_Speed_100MHz}, .sinit = 0}, //  ETH_RMII_RXD1

//     {.Base = GPIOF, {.GPIO_Pin = GPIO_Pin_9, .GPIO_Mode = GPIO_Mode_AN, .GPIO_PuPd = GPIO_PuPd_NOPULL, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},        //  ADC_CH0(ADC3_IN7)
//     {.Base = GPIOF, {.GPIO_Pin = GPIO_Pin_7, .GPIO_Mode = GPIO_Mode_AN, .GPIO_PuPd = GPIO_PuPd_NOPULL, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},        //  ADC_CH1(ADC3_IN5)
//     {.Base = GPIOF, {.GPIO_Pin = GPIO_Pin_6, .GPIO_Mode = GPIO_Mode_AN, .GPIO_PuPd = GPIO_PuPd_NOPULL, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},        //  ADC_CH2(ADC3_IN4)
//     {.Base = GPIOF, {.GPIO_Pin = GPIO_Pin_5, .GPIO_Mode = GPIO_Mode_AN, .GPIO_PuPd = GPIO_PuPd_NOPULL, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},        //  ADC_CH3(ADC3_IN15)
//     {.Base = GPIOF, {.GPIO_Pin = GPIO_Pin_4, .GPIO_Mode = GPIO_Mode_AN, .GPIO_PuPd = GPIO_PuPd_NOPULL, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},        //  ADC_CH4(ADC3_IN14)
//     {.Base = GPIOF, {.GPIO_Pin = GPIO_Pin_3, .GPIO_Mode = GPIO_Mode_AN, .GPIO_PuPd = GPIO_PuPd_NOPULL, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},        //  ADC_CH5(ADC3_IN9)
//     {.Base = GPIOF, {.GPIO_Pin = GPIO_Pin_10, .GPIO_Mode = GPIO_Mode_AN, .GPIO_PuPd = GPIO_PuPd_NOPULL, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},       //  ADC_CH6(ADC3_IN8)

//     {.Base = GPIOG, {.GPIO_Pin = GPIO_Pin_10, .GPIO_Mode = GPIO_Mode_IN, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},        // DIG_IN1-主板数字输入
//     {.Base = GPIOG, {.GPIO_Pin = GPIO_Pin_11, .GPIO_Mode = GPIO_Mode_IN, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},        // DIG_IN2
//     {.Base = GPIOG, {.GPIO_Pin = GPIO_Pin_12, .GPIO_Mode = GPIO_Mode_IN, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},        // DIG_IN3
//     {.Base = GPIOG, {.GPIO_Pin = GPIO_Pin_13, .GPIO_Mode = GPIO_Mode_IN, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},        // DIG_IN4
//     {.Base = GPIOB, {.GPIO_Pin = GPIO_Pin_9, .GPIO_Mode = GPIO_Mode_IN, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},         // DIG_IN5
//     {.Base = GPIOE, {.GPIO_Pin = GPIO_Pin_0, .GPIO_Mode = GPIO_Mode_IN, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},         // DIG_IN6
//     {.Base = GPIOF, {.GPIO_Pin = GPIO_Pin_1, .GPIO_Mode = GPIO_Mode_IN, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},         // DIG_IN1_C-扩展板数字输入
//     {.Base = GPIOF, {.GPIO_Pin = GPIO_Pin_13, .GPIO_Mode = GPIO_Mode_IN, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},        // DIG_IN2_C
//     {.Base = GPIOF, {.GPIO_Pin = GPIO_Pin_15, .GPIO_Mode = GPIO_Mode_IN, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},        // DIG_IN3_C
//     {.Base = GPIOG, {.GPIO_Pin = GPIO_Pin_1, .GPIO_Mode = GPIO_Mode_IN, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},         // DIG_IN4_C
//     {.Base = GPIOA, {.GPIO_Pin = GPIO_Pin_4, .GPIO_Mode = GPIO_Mode_IN, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},         // DIG_IN5_C
//     {.Base = GPIOF, {.GPIO_Pin = GPIO_Pin_2, .GPIO_Mode = GPIO_Mode_IN, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},         // DIG_IN6_C
//     {.Base = GPIOF, {.GPIO_Pin = GPIO_Pin_14, .GPIO_Mode = GPIO_Mode_IN, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},        // DIG_IN7_C
//     {.Base = GPIOG, {.GPIO_Pin = GPIO_Pin_0, .GPIO_Mode = GPIO_Mode_IN, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},         // DIG_IN8_C

//     {.Base = GPIOE, {.GPIO_Pin = GPIO_Pin_7,  .GPIO_Mode = GPIO_Mode_OUT, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 1},     // FAN1_OUT-控制风扇继电器输出
//     {.Base = GPIOE, {.GPIO_Pin = GPIO_Pin_8,  .GPIO_Mode = GPIO_Mode_OUT, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 1},     // FAN2_OUT-控制风扇继电器输出
//     {.Base = GPIOE, {.GPIO_Pin = GPIO_Pin_9,  .GPIO_Mode = GPIO_Mode_OUT, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 1},     // FAN3_OUT-控制风扇继电器输出
//     {.Base = GPIOE, {.GPIO_Pin = GPIO_Pin_12, .GPIO_Mode = GPIO_Mode_OUT, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 1},     // DIG_OUT1-预留第1路继电器输出
//     {.Base = GPIOE, {.GPIO_Pin = GPIO_Pin_13, .GPIO_Mode = GPIO_Mode_OUT, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 1},     // DIG_OUT2-预留第2路继电器输出
//     {.Base = GPIOE, {.GPIO_Pin = GPIO_Pin_14, .GPIO_Mode = GPIO_Mode_OUT, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 1},     // DIG_OUT3-预留第3路继电器输出
//     {.Base = GPIOE, {.GPIO_Pin = GPIO_Pin_1,  .GPIO_Mode = GPIO_Mode_OUT, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},     // DIG_OUT4
//      {.Base = GPIOB, {.GPIO_Pin = GPIO_Pin_0, .GPIO_Mode = GPIO_Mode_OUT, .GPIO_OType = GPIO_OType_PP, .GPIO_PuPd = GPIO_PuPd_UP, .GPIO_Speed = GPIO_Speed_50MHz}, .sinit = 0},      // DIG_OUT5
};


/***********************************************************************************
 * Function       : init_interface
 * Author         : Xu Shun'an
 * Date           : 2011.11.01
 * Description    : 端口初始化函数  
 * Calls          : None
 * Input          : None
 * Output         : None
 * Return         : None 
 ***********************************************************************************/ 
void init_interface(void)
{
    uint8_t tmp = sizeof(gGPIOAll)/sizeof(TGPIOA);

    for (uint8_t i = 0;i<tmp;++i){
        GPIO_Init(gGPIOAll[i].Base,(GPIO_InitTypeDef*)&gGPIOAll[i].io);
        if (1 == gGPIOAll[i].sinit){
            GPIO_ResetBits(gGPIOAll[i].Base,gGPIOAll[i].io.GPIO_Pin);
        }else if (2 == gGPIOAll[i].sinit){
            GPIO_SetBits(gGPIOAll[i].Base,gGPIOAll[i].io.GPIO_Pin);
        }
    }
	
    // 管脚映射
//     GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
//     GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
//     GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_USART2);
//     GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);

//     GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_CAN1);
//     GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_CAN1);
//     GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_CAN2);
//     GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_CAN2);
    
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SPI3);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_SPI3);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SPI3);

//     GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_ETH);
//     GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_ETH);
//     GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_ETH);
//     GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_ETH);
//     GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_ETH);
//     GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_ETH);
//     GPIO_PinAFConfig(GPIOC, GPIO_PinSource1, GPIO_AF_ETH);
//     GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_ETH);
//     GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_ETH);
}


