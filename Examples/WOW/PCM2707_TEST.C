#ifdef PCM2707_TEST
#include "PCM2707_TEST.H"
#include "PCM2707_TEST.H"

#include "STM32_SYS.H"
#include "STM32_GPIO.H"
#include "STM32_PWM.H"
#include "STM32_SPI.H"
#include "STM32_SYSTICK.H"
#include "STM32F10x_BitBand.H"


#include	"stdio.h"			//����printf
#include	"string.h"		//����printf
#include	"stdarg.h"		//���ڻ�ȡ��ȷ�������Ĳ���
#include	"stdlib.h"		//malloc��̬�����ڴ�ռ�

#include "EC11Encoder.H"

spi_def PCM2707_spi;
EC11_def	EC11_4;

EC11_status_def	status;

#define testlen 128
unsigned  char testrxbuffer[testlen]={0x05};
unsigned  char testtxbuffer[testlen]={0x05};

unsigned long time=	0;
unsigned char Flag=	0;


static void PCM2707_SPI_Configuration(void);
static void EC11_Configuration(void);
/*******************************************************************************
* ������		:	
* ��������	:	 
* ����		:	
* ���		:
* ���� 		:
*******************************************************************************/
void PCM2707_TEST_Configuration(void)
{
  SYS_Configuration();					//ϵͳ����---��ϵͳʱ�� STM32_SYS.H	
	PCM2707_SPI_Configuration();
	EC11_Configuration();
  SysTick_Configuration(10);
}

/*******************************************************************************
* ������		:	
* ��������	:	 
* ����		:	
* ���		:
* ���� 		:
*******************************************************************************/
void PCM2707_TEST_Server(void)
{	
	api_EC11_run_loop(&EC11_4);
	if(time<1000)
	{
		time++;
		return;
	}
	time=	0;
	status	=	api_EC11_get_status(&EC11_4);
	if(status.u8b.trigger_key)
	{
		api_spi_write_register_gpio(0x00,0x08);
		api_spi_write_register_gpio(0x00,0x00);
//		api_spi_write_register_gpio(0x00,0x02);
//		api_spi_write_register_gpio(0x00,0x00);
	}
	if(status.u8b.trigger_key)
	{
		api_spi_write_register_gpio(0x00,0x08);
		api_spi_write_register_gpio(0x00,0x00);
	}
	else if(status.u8b.trigger_right)
	{
		api_spi_write_register_gpio(0x00,0x02);
		api_spi_write_register_gpio(0x00,0x00);
	}
	else if(status.u8b.trigger_left)
	{
		api_spi_write_register_gpio(0x00,0x04);
		api_spi_write_register_gpio(0x00,0x00);
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
static void PCM2707_SPI_Configuration(void)
{
	PCM2707_spi.port.nss_port		=	GPIOC;
	PCM2707_spi.port.nss_pin		=	GPIO_Pin_6;
	PCM2707_spi.port.clk_port		=	GPIOC;
	PCM2707_spi.port.clk_pin		=	GPIO_Pin_7;
	PCM2707_spi.port.mosi_port	=	GPIOC;
	PCM2707_spi.port.mosi_pin		=	GPIO_Pin_8;
	
	api_spi_configuration_gpio(&PCM2707_spi);
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
static void EC11_Configuration(void)
{
	EC11_4.port.button_port	=	GPIOB;
	EC11_4.port.button_pin	=	GPIO_Pin_4;
	
	EC11_4.port.A_port			=	GPIOB;
	EC11_4.port.A_pin				=	GPIO_Pin_3;
	
	EC11_4.port.B_port			=	GPIOD;
	EC11_4.port.B_pin				=	GPIO_Pin_2;
	
	EC11_4.statictime.ScanTime	=	10;
	api_EC11_configuration_gpio(&EC11_4);
}



#endif