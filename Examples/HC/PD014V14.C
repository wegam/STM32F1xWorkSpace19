/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : PC001V21.c
* Author             : WOW
* Version            : V2.0.1
* Date               : 06/26/2017
* Description        : PC001V21层控制板.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

#ifdef PD014V14			//盒剂发药控制板

#include "PD014V14.H"

#include "DRV8801.H"
#include "A3987.H"

#include "STM32F10x_BitBand.H"
#include "STM32_GPIO.H"
#include "STM32_SYS.H"
#include "STM32_SYSTICK.H"
#include "STM32_WDG.H"
#include "STM32_PWM.H"
#include "STM32_USART.H"
#include "STM32_TOOLS.H"

#include "string.h"				//串和内存操作函数头文件
#include "stm32f10x_dma.h"




u16	DelayTime=0;
u16	StepTime=0;

u16 Lock_Toggle_CNT=0;
PD014_INF_TypeDef	PD014_Conf;
RS485_TypeDef	PD014R485;

/*******************************************************************************
* 函数名		:	
* 功能描述	:	 
* 输入		:	
* 输出		:
* 返回 		:
*******************************************************************************/
void PD014V14_Configuration(void)
{
	SYS_Configuration();					//系统配置---打开系统时钟 STM32_SYS.H
	
	GPIO_DeInitAll();							//将所有的GPIO关闭----V20170605
	
	PD014V14_PinSet();
	
	RS485_DMA_ConfigurationNR	(&PD014R485,19200,(u32*)PD014_Conf.PD014_DATA.RxdBuffe,RS485BufferSize);	//USART_DMA配置--查询方式，不开中断,配置完默认为接收状态
	
	SysTick_Configuration(10);	//系统嘀嗒时钟配置72MHz,单位为uS--1ms
	
	IWDG_Configuration(1000);			//独立看门狗配置---参数单位ms	
	
	PWM_OUT(TIM2,PWM_OUTChannel1,0.5,950);						//PWM设定-20161127版本--SYS-LED
	
	PD014V14_GetSwitchID();				//获取拨码开关地址
	
	PD014V14_GetOnlieDevice();			//获取在线发药头
}
/*******************************************************************************
* 函数名		:	
* 功能描述	:	 
* 输入		:	
* 输出		:
* 返回 		:
*******************************************************************************/
void PD014V14_Server(void)
{	
	u8 RxNum=0;
	
	IWDG_Feed();								//独立看门狗喂狗
	
	if(PD014_Conf.PD014_DATA.DeviceOnlie==0x00)
	{
		if(PD014_Conf.PD014_DATA.TimeCount++>=1000000)		//等待100mS
		{
			PD014_Conf.PD014_DATA.TimeCount=0;
			PD014V14_GetOnlieDevice();			//获取在线发药头
		}
	}
	else
	{
		//数据格式:B0-SWITCHID，B1-CMD，B2~B9:数据，B10:前面所有数据异或校验
		RxNum=RS485_ReadBufferIDLE(&PD014R485,(u32*)PD014_Conf.PD014_DATA.RevBuffe,(u32*)PD014_Conf.PD014_DATA.RxdBuffe);	//串口空闲模式读串口接收缓冲区，如果有数据，将数据拷贝到RevBuffer,并返回接收到的数据个数，然后重新将接收缓冲区地址指向RxdBuffer
		if(RxNum&&(PD014_Conf.PD014_DATA.RevBuffe[0]==PD014_Conf.PD014_DATA.SWITCHID))		//如果拨码地址相同，则进入Process
		{
			unsigned char Bcc=BCC8(PD014_Conf.PD014_DATA.RevBuffe,10);		//异或校验;
			if(Bcc==PD014_Conf.PD014_DATA.RevBuffe[10])			//异或校验通过
			{
				PD014V14_Process();		//PD014V14所有板内处理数理函数
			}
			else
			{
				PD014_Conf.PD014_DATA.TxdBuffe[0]=PD014_NACK;
				PD014_Conf.PD014_DATA.TxdBuffe[1]=PD014_BccError;
				PD014_Conf.PD014_DATA.TxdBuffe[2]=PD014_NACK^PD014_BccError;
				PD014V14_NACK();			//数据校验不通过，NACK应答主机
			}
		}
	}
	PD014V14_SendM();			//发药
//	Lock_Toggle();			//双向电子锁控制
}
/*******************************************************************************
* 函数名			:	PD014V14_Process
* 功能描述		:	PD014V14_Process 
* 输入			: void
* 返回值			: void
*******************************************************************************/
void PD014V14_Process(void)		//PD014V14所有板内处理数理函数
{
	//判断数据命令类型;数据格式:B0-SWITCHID，B1-CMD，B2~B9:数据，B10:前面所有数据异或校验
	PD014_CMD_TypeDef CMD_Temp=PD014_CMD_IDLE;		//临时命令变量
	CMD_Temp=(PD014_CMD_TypeDef)PD014_Conf.PD014_DATA.RevBuffe[1];		//获取数据命令
	if(CMD_Temp==PD014_CMD_GetStatus)
	{
		PD014_Conf.PD014_DATA.TxdBuffe[0]=PD014_CMD_GetStatus;
		PD014_Conf.PD014_DATA.TxdBuffe[1]=PD014_Conf.PD014_DATA.SWITCHID;
		memcpy(&(PD014_Conf.PD014_DATA.TxdBuffe[2]),PD014_Conf.PD014_DATA.STATUS,8);
		PD014_Conf.PD014_DATA.TxdBuffe[10]=BCC8(PD014_Conf.PD014_DATA.TxdBuffe,10);		//异或校验;
		RS485_DMASend(&PD014R485,(u32*)PD014_Conf.PD014_DATA.TxdBuffe,11);						//RS485-DMA发送程序
	}
	else if(memchr(PD014_Conf.PD014_DATA.STATUS,PD014_STA_IDLE,8)==NULL)		//非空闲状态
	{
		PD014_Conf.PD014_DATA.TxdBuffe[0]=PD014_NACK;
		PD014_Conf.PD014_DATA.TxdBuffe[1]=PD014_BUSY;
		PD014_Conf.PD014_DATA.TxdBuffe[2]=PD014_NACK^PD014_BUSY;
		PD014V14_NACK();		//NACK返回3字节，NACK标识、错误内容、BCC校验码		
	}
	else if(CMD_Temp==PD014_CMD_SetWSD)		//发药命令---传入需要发药的数量
	{
		unsigned char i=0;
		memcpy(&(PD014_Conf.PD014_DATA.WSD),&(PD014_Conf.PD014_DATA.RevBuffe[2]),8);
		for(i=0;i<8;i++)
		{
			if(PD014_Conf.PD014_DATA.WSD[i])
			{
				PD014_Conf.PD014_DATA.STATUS[i]=PD014_STA_SEND;
				PD014_Conf.PD014_DATA.RELAYTIME[i]=0;
				PD014_Conf.PD014_DATA.RELAYCOUNT[i]=0;
				PD014_Conf.PD014_DATA.SENSTIME[i]=0;
				PD014_Conf.PD014_DATA.NOISETIME[i]=0;
			}
			else
			{
				PD014_Conf.PD014_DATA.STATUS[i]=PD014_STA_IDLE;
			}
		}
	}		
}

/*******************************************************************************
* 函数名		:	
* 功能描述	:	 
* 输入		:	
* 输出		:
* 返回 		:
*******************************************************************************/
void PD014V14_PinSet(void)
{	
	//发药电磁铁控制
	GPIO_Configuration_OPP50	(GPIOA,	GPIO_Pin_7);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	GPIO_Configuration_OPP50	(GPIOC,	GPIO_Pin_5);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	GPIO_Configuration_OPP50	(GPIOB,	GPIO_Pin_1);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	GPIO_Configuration_OPP50	(GPIOB,	GPIO_Pin_11);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	GPIO_Configuration_OPP50	(GPIOB,	GPIO_Pin_13);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	GPIO_Configuration_OPP50	(GPIOB,	GPIO_Pin_15);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	GPIO_Configuration_OPP50	(GPIOC,	GPIO_Pin_7);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	GPIO_Configuration_OPP50	(GPIOC,	GPIO_Pin_9);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	
	//发药传感器供电开关
	GPIO_Configuration_OPP50	(GPIOA,	GPIO_Pin_5);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	
	//发药传感器
	GPIO_Configuration_IPU(GPIOA,	GPIO_Pin_6);					//将GPIO相应管脚配置为上拉输入模式----V20170605
	GPIO_Configuration_IPU(GPIOC,	GPIO_Pin_4);					//将GPIO相应管脚配置为上拉输入模式----V20170605
	GPIO_Configuration_IPU(GPIOB,	GPIO_Pin_0);					//将GPIO相应管脚配置为上拉输入模式----V20170605
	GPIO_Configuration_IPU(GPIOB,	GPIO_Pin_10);					//将GPIO相应管脚配置为上拉输入模式----V20170605
	GPIO_Configuration_IPU(GPIOB,	GPIO_Pin_12);					//将GPIO相应管脚配置为上拉输入模式----V20170605
	GPIO_Configuration_IPU(GPIOB,	GPIO_Pin_14);					//将GPIO相应管脚配置为上拉输入模式----V20170605
	GPIO_Configuration_IPU(GPIOC,	GPIO_Pin_6);					//将GPIO相应管脚配置为上拉输入模式----V20170605
	GPIO_Configuration_IPU(GPIOC,	GPIO_Pin_8);					//将GPIO相应管脚配置为上拉输入模式----V20170605	
	
	//拨码开关
	GPIO_Configuration_IPU(GPIOB,	GPIO_Pin_7);					//将GPIO相应管脚配置为上拉输入模式----V20170605
	GPIO_Configuration_IPU(GPIOB,	GPIO_Pin_6);					//将GPIO相应管脚配置为上拉输入模式----V20170605
	GPIO_Configuration_IPU(GPIOB,	GPIO_Pin_5);					//将GPIO相应管脚配置为上拉输入模式----V20170605
	GPIO_Configuration_IPU(GPIOB,	GPIO_Pin_4);					//将GPIO相应管脚配置为上拉输入模式----V20170605
	GPIO_Configuration_IPU(GPIOB,	GPIO_Pin_3);					//将GPIO相应管脚配置为上拉输入模式----V20170605
	GPIO_Configuration_IPU(GPIOC,	GPIO_Pin_12);					//将GPIO相应管脚配置为上拉输入模式----V20170605
	GPIO_Configuration_IPU(GPIOC,	GPIO_Pin_11);					//将GPIO相应管脚配置为上拉输入模式----V20170605
	GPIO_Configuration_IPU(GPIOC,	GPIO_Pin_10);					//将GPIO相应管脚配置为上拉输入模式----V20170605
	
	PD014R485.USARTx=USART2;
	PD014R485.RS485_CTL_PORT=GPIOA;
	PD014R485.RS485_CTL_Pin=GPIO_Pin_1;
	
	P_Sens=1;			//传感器供电开关--开
}
/*******************************************************************************
* 函数名			:	function
* 功能描述		:	函数功能说明 
* 输入			: void
* 返回值			: void
*******************************************************************************/
void Lock_Toggle(void)
{
	Lock_Toggle_CNT++;
	if(Lock_Toggle_CNT>=2000)
	{
		Lock_Toggle_CNT=0;
		ct_pmos1=0;
		ct_pmos2=0;
		ct_pmos3=0;
		ct_pmos4=0;
		ct_pmos5=0;
		ct_pmos6=0;
		ct_pmos7=0;
		ct_pmos8=0;
	}
	else if(Lock_Toggle_CNT==100)
	{
		if(Sens_In1)
			ct_pmos1=1;
		if(Sens_In2)
			ct_pmos2=1;
		if(Sens_In3)
			ct_pmos3=1;
		if(Sens_In4)
			ct_pmos4=1;
		if(Sens_In5)
			ct_pmos5=1;
		if(Sens_In6)
			ct_pmos6=1;
		if(Sens_In7)
			ct_pmos7=1;
		if(Sens_In8)
			ct_pmos8=1;
		
	}
//	else if(Lock_Toggle_CNT==200)
//	{
//		ct_pmos1=0;
//		ct_pmos2=0;
//		ct_pmos3=0;
//		ct_pmos4=0;
//		ct_pmos5=0;
//		ct_pmos6=0;
//		ct_pmos7=0;
//		ct_pmos8=0;		
//	}
	else if(Lock_Toggle_CNT>=400)
	{
		ct_pmos1=0;
		ct_pmos2=0;
		ct_pmos3=0;
		ct_pmos4=0;
		ct_pmos5=0;
		ct_pmos6=0;
		ct_pmos7=0;
		ct_pmos8=0;
	}
}
/*******************************************************************************
* 函数名			:	PD014V14_GetSwitchID
* 功能描述		:	获取拨码开关地址 
* 输入			: void
* 返回值			: void
*******************************************************************************/
void PD014V14_GetSwitchID(void)				//获取拨码开关地址
{
	u8 tempID=0;
	tempID=tempID<<1;
	tempID+=PD014_Switch8;		//S8-MSB
	tempID=tempID<<1;
	tempID+=PD014_Switch7;		//
	tempID=tempID<<1;
	tempID+=PD014_Switch6;		//
	tempID=tempID<<1;
	tempID+=PD014_Switch5;		//
	tempID=tempID<<1;
	tempID+=PD014_Switch4;		//
	tempID=tempID<<1;
	tempID+=PD014_Switch3;		//
	tempID=tempID<<1;
	tempID+=PD014_Switch2;		//
	tempID=tempID<<1;
	tempID+=PD014_Switch1;		//
	tempID=tempID^0xFF;
	
	
	if(PD014_Conf.PD014_DATA.SWITCHID&0x80)	//最高位为1，则CAN开启
	{
		PD014_Conf.PD014_DATA.CANFLG=1;
	}
	if(PD014_Conf.PD014_DATA.SWITCHID&0x7F)	//最高位为1，则CAN开启
	{
		PD014_Conf.MODE=NORMEL;					//有拨码，正常模式
	}
	else
	{
		PD014_Conf.MODE=TEST;						//无拨码，测试模式
	}
	
	PD014_Conf.PD014_DATA.SWITCHID=tempID;
}
/*******************************************************************************
* 函数名			:	PD014V14_GetOnlieDevice
* 功能描述		:	函数功能说明 
* 输入			: void
* 返回值			: void
*******************************************************************************/
void PD014V14_GetOnlieDevice(void)			//获取在线发药头
{
	u8 OnLD=0;		//DeviceOnlie
	if(Sens_In1)
		OnLD+=1;
	OnLD<<=1;
	if(Sens_In2)
		OnLD+=1;
	OnLD<<=1;
	if(Sens_In3)
		OnLD+=1;
	OnLD<<=1;
	if(Sens_In4)
		OnLD+=1;
	OnLD<<=1;
	if(Sens_In5)
		OnLD+=1;
	OnLD<<=1;
	if(Sens_In6)
		OnLD+=1;
	OnLD<<=1;
	if(Sens_In7)
		OnLD+=1;
	OnLD<<=1;
	if(Sens_In8)
		OnLD+=1;
	OnLD=OnLD^0xFF;
	PD014_Conf.PD014_DATA.DeviceOnlie=OnLD;
}

/*******************************************************************************
* 函数名			:	function
* 功能描述		:	函数功能说明 
* 输入			: void
* 返回值			: void
*******************************************************************************/
void PD014V14_ACK(void)
{
	PD014_Conf.PD014_DATA.TxdBuffe[0]=PD014_ACK;
	PD014_Conf.PD014_DATA.TxdBuffe[1]=PD014_ACK^0xFF;
	RS485_DMASend(&PD014R485,(u32*)PD014_Conf.PD014_DATA.TxdBuffe,2);	//RS485-DMA发送程序
}
/*******************************************************************************
* 函数名			:	function
* 功能描述		:	函数功能说明 
* 输入			: void
* 返回值			: void
*******************************************************************************/
void PD014V14_NACK(void)		//NACK返回3字节，NACK标识、错误内容、BCC校验码
{
//	PD014_Conf.PD014_DATA.TxdBuffe[0]=PD014_NACK;
//	PD014_Conf.PD014_DATA.TxdBuffe[1]=PD014_NACK^0xFF;
	RS485_DMASend(&PD014R485,(u32*)PD014_Conf.PD014_DATA.TxdBuffe,3);	//RS485-DMA发送程序
}
/*******************************************************************************
* 函数名			:	function
* 功能描述		:	函数功能说明 
* 输入			: void
* 返回值			: void
*******************************************************************************/
void PD014V14_SendM(void)		//发药
{
	unsigned char i=0;
	for(i=0;i<8;i++)
	{
		if(PD014_Conf.PD014_DATA.STATUS[i]==PD014_STA_SEND)
		{
			if(i==1)
				ct_pmos1=1;
			else if(i==2)
				ct_pmos2=1;
			else if(i==3)
				ct_pmos3=1;
			else if(i==4)
				ct_pmos4=1;
			else if(i==5)
				ct_pmos5=1;
			else if(i==6)
				ct_pmos6=1;
			else if(i==7)
				ct_pmos7=1;
			else if(i==8)
				ct_pmos8=1;
		}
		else
		{
			if(i==1)
				ct_pmos1=0;
			else if(i==2)
				ct_pmos2=0;
			else if(i==3)
				ct_pmos3=0;
			else if(i==4)
				ct_pmos4=0;
			else if(i==5)
				ct_pmos5=0;
			else if(i==6)
				ct_pmos6=0;
			else if(i==7)
				ct_pmos7=0;
			else if(i==8)
				ct_pmos8=0;
		}
	}
}
/*******************************************************************************
* 函数名			:	PD014V14_ResetData
* 功能描述		:	复位数据 
* 输入			: void
* 返回值			: void
*******************************************************************************/
void PD014V14_ResetData(void)					//复位数据
{
	PD014_Conf.PD014_DATA.CANFLG=0;				//CAN开关---CANFLG==1，使用CAN--为与上层连接的板
	PD014_Conf.PD014_DATA.SWITCHID=0;			//拨码开关
	PD014_Conf.PD014_DATA.DeviceOnlie=0;	//发药头标志---存储是否接入发药头，有，当前位为1，否则为0	//1(最左边为最高位)
	
	PD014_Conf.PD014_DATA.TimeCount=0;		//运行次数计时器
	PD014_Conf.PD014_DATA.TimeOut=0;			//支行超时计时器
	
	memset(PD014_Conf.PD014_DATA.WSD,0x00,8);		//待发药数量
	memset(PD014_Conf.PD014_DATA.SED,0x00,8);		//已发药数量	
	
	memset(PD014_Conf.PD014_DATA.RELAYTIME,0x64,8);		//继电器需要吸合时间---默认100ms(1ms*100)
	memset(PD014_Conf.PD014_DATA.RELAYCOUNT,0x00,8);	//继电器已经吸合时间---达到RELAYTIME后释放
	
	memset(PD014_Conf.PD014_DATA.SENSTIME,0x00,8);		//传感器感应时间----与NOISETIME配置使用计算传感器感应时间，主要过滤干扰信号
	memset(PD014_Conf.PD014_DATA.NOISETIME,0x00,8);		//传感器感噪声过滤时间	
	
	memset(PD014_Conf.PD014_DATA.RxdBuffe,0x00,8);
	memset(PD014_Conf.PD014_DATA.RevBuffe,0x00,8);
	memset(PD014_Conf.PD014_DATA.TxdBuffe,0x00,8);
	memset(PD014_Conf.PD014_DATA.TevBuffe,0x00,8);
	
	PD014_Conf.PD014_DATA.Command=PD014_CMD_IDLE;			//命令
	memset(PD014_Conf.PD014_DATA.STATUS,PD014_STA_IDLE,8);	//状态	
}




#endif
