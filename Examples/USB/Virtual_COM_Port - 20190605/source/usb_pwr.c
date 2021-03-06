/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : usb_pwr.c
* Author             : MCD Application Team
* Version            : V2.2.0
* Date               : 06/13/2008
* Description        : Connection/disconnection & power management
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/


/******************************** 说明20160912**********************************
********************************************************************************
* 功能：用于USB连接、断开操作
* 
* 
* 
* 
* 
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_lib.h"
#include "usb_lib.h"
#include "usb_conf.h"
#include "usb_pwr.h"
//#include "hw_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
DEVICE_STATE bDeviceState = UNCONNECTED; 		/* USB device status */	//USB设备当前状态
volatile bool fSuspendEnabled = TRUE;  /* True when suspend is possible */

struct
{
  volatile RESUME_STATE eState;
  volatile u8 bESOFcnt;
}
ResumeS;

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : PowerOn
* Description    : USB IP power ON Routine.
* Input          : None.
* Output         : None.
* Return         : USB_SUCCESS.
*******************************************************************************/
RESULT PowerOn(void)		//处理开关开情形
{
  u16 wRegVal;

  /*** cable plugged-in ? ***/
  usb_hw_set_connect(ENABLE);			//使能1.5K上拉

  /*** CNTR_PWDN = 0 ***/
  wRegVal = CNTR_FRES;					//强制USB复位
  _SetCNTR(wRegVal);						//设置控制寄存器

  /*** CNTR_FRES = 0 ***/
  wInterrupt_Mask = 0;
  _SetCNTR(wInterrupt_Mask);
  /*** Clear pending interrupts ***/
  _SetISTR(0);									//清除状态寄存器
  /*** Set interrupt mask ***/
  wInterrupt_Mask = CNTR_RESETM | CNTR_SUSPM | CNTR_WKUPM;		//允许以下中断		
													//CNTR_RESETM：USB复位中断屏蔽位 (USB reset interrupt mask)
													//CNTR_SUSPM：挂起中断屏蔽位 (Suspend mode interrupt mask)
													//CNTR_WKUPM：唤醒中断屏蔽位 (Wakeup interrupt mask)
  _SetCNTR(wInterrupt_Mask);	//使能相应中断

  return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : PowerOff
* Description    : Handles switch-off conditions
* Input          : None.
* Output         : None.
* Return         : USB_SUCCESS.
*******************************************************************************/
RESULT PowerOff()			//处理开关关情形
{
  /* disable all ints and force USB reset */
  _SetCNTR(CNTR_FRES);
  /* clear interrupt status register */
  _SetISTR(0);
  /* Disable the Pull-Up*/
  usb_hw_set_connect(DISABLE);		//关闭1.5K上拉，断开USB
  /* switch-off device */
  _SetCNTR(CNTR_FRES + CNTR_PDWN);
  /* sw variables reset */
  /* ... */

  return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : Suspend
* Description    : sets suspend mode operating conditions--挂起
* Input          : None.
* Output         : None.
* Return         : USB_SUCCESS.
*******************************************************************************/
void Suspend(void)		//设置挂起模式运行清醒
{
  u16 wCNTR;
  /* suspend preparation */
  /* ... */

  /* macrocell enters suspend mode */
  wCNTR = _GetCNTR();
  wCNTR |= CNTR_FSUSP;
  _SetCNTR(wCNTR);

  /* ------------------ ONLY WITH BUS-POWERED DEVICES ---------------------- */
  /* power reduction */
  /* ... on connected devices */


  /* force low-power mode in the macrocell */
  wCNTR = _GetCNTR();
  wCNTR |= CNTR_LPMODE;
  _SetCNTR(wCNTR);

  /* switch-off the clocks */
  /* ... */
  Enter_LowPowerMode();

}

/*******************************************************************************
* Function Name  : Resume_Init
* Description    : Handles wake-up restoring normal operations 唤醒请求
* Input          : None.
* Output         : None.
* Return         : USB_SUCCESS.
*******************************************************************************/
void Resume_Init(void)
{
  u16 wCNTR;
  /* ------------------ ONLY WITH BUS-POWERED DEVICES ---------------------- */
  /* restart the clocks */
  /* ...  */

  /* CNTR_LPMODE = 0 */
  wCNTR = _GetCNTR();
  wCNTR &= (~CNTR_LPMODE);
  _SetCNTR(wCNTR);

  /* restore full power */
  /* ... on connected devices */
  Leave_LowPowerMode();

  /* reset FSUSP bit */
  _SetCNTR(IMR_MSK);

  /* reverse suspend preparation */
  /* ... */

}

/*******************************************************************************
* Function Name  : Resume
* Description    : This is the state machine handling resume operations and
*                 timing sequence. The control is based on the Resume structure
*                 variables and on the ESOF interrupt calling this subroutine
*                 without changing machine state.
* Input          : a state machine value (RESUME_STATE)
*                  RESUME_ESOF doesn't change ResumeS.eState allowing
*                  decrementing of the ESOF counter in different states.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Resume(RESUME_STATE eResumeSetVal)		//处理唤醒操作
{
  u16 wCNTR;

  if (eResumeSetVal != RESUME_ESOF)
    ResumeS.eState = eResumeSetVal;

  switch (ResumeS.eState)
  {
    case RESUME_EXTERNAL:
      Resume_Init();
      ResumeS.eState = RESUME_OFF;
      break;
    case RESUME_INTERNAL:
      Resume_Init();
      ResumeS.eState = RESUME_START;
      break;
    case RESUME_LATER:
      ResumeS.bESOFcnt = 2;
      ResumeS.eState = RESUME_WAIT;
      break;
    case RESUME_WAIT:
      ResumeS.bESOFcnt--;
      if (ResumeS.bESOFcnt == 0)
        ResumeS.eState = RESUME_START;
      break;
    case RESUME_START:
      wCNTR = _GetCNTR();
      wCNTR |= CNTR_RESUME;
      _SetCNTR(wCNTR);
      ResumeS.eState = RESUME_ON;
      ResumeS.bESOFcnt = 10;
      break;
    case RESUME_ON:
      ResumeS.bESOFcnt--;
      if (ResumeS.bESOFcnt == 0)
      {
        wCNTR = _GetCNTR();
        wCNTR &= (~CNTR_RESUME);
        _SetCNTR(wCNTR);
        ResumeS.eState = RESUME_OFF;
      }
      break;
    case RESUME_OFF:
    case RESUME_ESOF:
    default:
      ResumeS.eState = RESUME_OFF;
      break;
  }
}
/*******************************************************************************
* Function Name  : Enter_LowPowerMode
* Description    : Power-off system clocks and power while entering suspend mode
* 描述			    	: 挂起模式
* Input          : None.
* Return         : None.
*******************************************************************************/
void Enter_LowPowerMode(void)
{
  /* Set the device state to suspend */
  bDeviceState = SUSPENDED;
}

/*******************************************************************************
* Function Name  : Leave_LowPowerMode
* Description    : Restores system clocks and power while exiting suspend mode
* 描述			    	: 退出挂起模式
* Input          : None.
* Return         : None.
*******************************************************************************/
void Leave_LowPowerMode(void)
{
	DEVICE_INFO *pInfo = pInformation;
  /* Set the device state to the correct state */
  if (pInfo->Current_Configuration != 0)
  {
    /* Device configured */
    bDeviceState = CONFIGURED;
  }
  else
  {
    bDeviceState = ATTACHED;
  }
}
/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
