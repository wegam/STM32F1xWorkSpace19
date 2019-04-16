#ifdef AMP01V11A2			//50PCS��Ʒ�汾

#include "AMP01V11A2.H"


//#include "AMP_LAY.H"
#include "AMP_CABV11A2.H"

#include	"AMP_PHY.H"

#include "STM32F10x_BitBand.H"
#include "STM32_GPIO.H"
#include "STM32_SYS.H"
#include "STM32_SYSTICK.H"
#include "STM32_WDG.H"
#include "STM32_PWM.H"
#include "STM32_USART.H"

#include "SWITCHID.H"
#include "IOT5302W.H"     //������

#include 	"CRC.H"

#include "string.h"				//�����ڴ��������ͷ�ļ�
/* Private variables ---------------------------------------------------------*/
static RS485Def stCbRS485Ly;   //uart4,PA15   //���ӿ�
static RS485Def stCbRS485Cb;   //usart1,PA8    //����ӿ�
static RS485Def stCardRS485Ly; //usart3,PB2    //�������ӿ�
static SwitchDef stCbSwitch;
//tAMPProdef   AMPPro;
/* Private function prototypes -----------------------------------------------*/
static void Hardware_Configuration(void);
static void HW_SwitchID_Configuration(void);
/*******************************************************************************
*������			:	function
*��������		:	function
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	wegam@sina.com
*******************************************************************************/
void AMP01V11A2_Configuration(void)
{	

	AMP_CABV11_Configuration();

  GPIO_Configuration_OPP50(SYSLEDPort,SYSLEDPin);

	//SysTick_DeleymS(500);				//SysTick��ʱnmS
	
  IWDG_Configuration(2000);			//�������Ź�����---������λms
  
  SysTick_Configuration(1000);	//ϵͳ���ʱ������72MHz,��λΪuS
  
  while(1)
  {

  }
}
/*******************************************************************************
*������			:	function
*��������		:	function
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	wegam@sina.com
*******************************************************************************/
void AMP01V11A2_Server(void)
{  
	IWDG_Feed();								//�������Ź�ι��
	Tim_Server();

	AMP_CABV11_Server();
}
/*******************************************************************************
*������			:	function
*��������		:	function
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	wegam@sina.com
*******************************************************************************/
static void Hardware_Configuration(void)
{
  HW_SwitchID_Configuration();
	//=======================================================���뿪������
	  
  AMPCAB_GenyConfiguration();   //����ӿ����ã����⣬������Դ����
    
  AMPCABCOMM_Configuration();   //ͨѶ����
  
  SysTick_DeleymS(1000);				//SysTick��ʱnmS--���ϵ��ȶ�
}
/*******************************************************************************
* ������			:	function
* ��������		:	��������˵�� 
* ����			: void
* ����ֵ			: void
* �޸�ʱ��		: ��
* �޸�����		: ��
* ����			: wegam@sina.com
*******************************************************************************/
static void HW_SwitchID_Configuration(void)
{
  stCbSwitch.NumOfSW	=	8;
  
  stCbSwitch.SW1_PORT	=	GPIOB;
  stCbSwitch.SW1_Pin	=	GPIO_Pin_9;
  
  stCbSwitch.SW2_PORT	=	GPIOB;
  stCbSwitch.SW2_Pin	=	GPIO_Pin_8;
  
  stCbSwitch.SW3_PORT	=	GPIOB;
  stCbSwitch.SW3_Pin	=	GPIO_Pin_7;
  
  stCbSwitch.SW4_PORT	=	GPIOB;
  stCbSwitch.SW4_Pin	=	GPIO_Pin_6;
  
  stCbSwitch.SW5_PORT	=	GPIOB;
  stCbSwitch.SW5_Pin	=	GPIO_Pin_5;
  
  stCbSwitch.SW6_PORT	=	GPIOB;
  stCbSwitch.SW6_Pin	=	GPIO_Pin_4;
  
  stCbSwitch.SW7_PORT	=	GPIOB;
  stCbSwitch.SW7_Pin	=	GPIO_Pin_3;
  
  stCbSwitch.SW8_PORT	=	GPIOD;
  stCbSwitch.SW8_Pin	=	GPIO_Pin_2;

	SwitchIdInitialize(&stCbSwitch);						//

//  CabAddr  = SWITCHID_ReadLeft(&stCbSwitch)&0x3F;  
//  
//  if(SWITCHID_ReadLeft(&stCbSwitch)&0x80)
//  {
//    MainFlag=1; //0--����1--����
//  }
//  else
//  {
//    MainFlag=0; //0--����1--����
//  }
}








//=================================�����ӿ�End=============================================================



/*******************************************************************************
*������			:	function
*��������		:	function
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	wegam@sina.com
*******************************************************************************/
unsigned short HW_SendBuff(enCCPortDef Port,unsigned char* pBuffer,unsigned short length)
{ 
  unsigned  short   sendedlen = 0;

    sendedlen = AMPCAB_SendBuff(Port,pBuffer,length);

  return  sendedlen;
}




//=================================Ӳ���ӿ�End=============================================================


//=================================���ú���End=============================================================


//=================================ʱ�亯��ST==============================================================

/*******************************************************************************
*������			:	function
*��������		:	function
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	wegam@sina.com
*******************************************************************************/
void Tim_Server(void)
{
  //----------------PC����
  if(AMPPro.Time.PcSendTime>0)
  {
    AMPPro.Time.PcSendTime--;
  }
  //----------------����
  if(AMPPro.Time.CabSendTime>0)
  {
    AMPPro.Time.CabSendTime--;
  }
  //----------------�㷢��
  if(AMPPro.Time.LaySendTime>0)
  {
    AMPPro.Time.LaySendTime--;
  }
  //----------------����������
  if(AMPPro.Time.CardSendTime>0)
  {
    AMPPro.Time.CardSendTime--;
  }
  //----------------��
  if(AMPPro.Time.LockTime>0)
  {
    AMPPro.Time.LockTime--;
  }
  //----------------����
  if(AMPPro.Time.swicthidtime>0)
  {
    AMPPro.Time.swicthidtime--;
  }
  //----------------����ָʾ��
  if(AMPPro.Time.SYSLEDTime>0)
  {
    AMPPro.Time.SYSLEDTime--;
  }
}
#endif