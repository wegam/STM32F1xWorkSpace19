/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : platform_config.h
* Author             : MCD Application Team
* Version            : V2.2.0
* Date               : 06/13/2008
* Description        : Evaluation board specific configuration file.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_type.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Uncomment the line corresponding to the STMicroelectronics evaluation board
   used to run the example */
#define lora	0
#define STM32_FSMC 1
#define usart_buffer_size	1024

#if !defined (USE_STM3210B_EVAL) &&  !defined (USE_STM3210E_EVAL)
 //#define USE_STM3210B_EVAL
 #define USE_STM3210E_EVAL
#endif

/* Define the STM32F10x hardware depending on the used evaluation board */
#ifdef USE_STM3210B_EVAL

  #define USB_DISCONNECT            GPIOD  
  #define USB_DISCONNECT_PIN        GPIO_Pin_9
  #define RCC_APB2Periph_GPIO_DISCONNECT      RCC_APB2Periph_GPIOD

#else /* USE_STM3210E_EVAL */

	#ifdef PS005
		#define USB_DISCONNECT            GPIOA  
		#define USB_DISCONNECT_PIN        GPIO_Pin_8
		#define RCC_APB2Periph_GPIO_DISCONNECT      RCC_APB2Periph_GPIOA
	#elif lora
	  #define USB_DISCONNECT            GPIOA  
		#define USB_DISCONNECT_PIN        GPIO_Pin_15
		#define RCC_APB2Periph_GPIO_DISCONNECT      RCC_APB2Periph_GPIOA
	#elif STM32_FSMC
	  #define USB_DISCONNECT            GPIOA  
		#define USB_DISCONNECT_PIN        GPIO_Pin_8
		#define RCC_APB2Periph_GPIO_DISCONNECT      RCC_APB2Periph_GPIOA
	#else
	  #define USB_DISCONNECT            GPIOA  
		#define USB_DISCONNECT_PIN        GPIO_Pin_8
		#define RCC_APB2Periph_GPIO_DISCONNECT      RCC_APB2Periph_GPIOA
	#endif

#endif /* USE_STM3210B_EVAL */

#define ComPort	USART1

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __PLATFORM_CONFIG_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
