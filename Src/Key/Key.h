/**
  ******************************************************************************
  * @file    :
  * @author  :
  * @version :
  * @date    :
  * @brief   :
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef KEY__
#define KEY__

/******************************************************************************
Includes
******************************************************************************/
#include "stm32f4xx_hal.h"

/******************************************************************************
Definitions
******************************************************************************/
/* Key bit mask */
#define BIT_MASK_KEY_MAP			0x80
#define BIT_MASK_KEY_MENU			0x40
#define BIT_MASK_KEY_BACK			0x20
#define BIT_MASK_KEY_UP				0x10

/* Key pin configuration */
#define KEY_MAP_PIN				GPIO_PIN_10
#define KEY_MAP_PORT			GPIOC
#define KEY_MAP_CLK_EN()		__GPIOC_CLK_ENABLE()

#define KEY_MENU_PIN			GPIO_PIN_11
#define KEY_MENU_PORT			GPIOC
#define KEY_MENU_CLK_EN()		__GPIOC_CLK_ENABLE()

#define KEY_BACK_PIN			GPIO_PIN_12
#define KEY_BACK_PORT			GPIOC
#define KEY_BACK_CLK_EN()		__GPIOC_CLK_ENABLE()

#define KEY_UP_PIN				GPIO_PIN_0
#define KEY_UP_PORT				GPIOB
#define KEY_UP_CLK_EN()			__GPIOC_CLK_ENABLE()

/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/
void Key_Init(void);
uint8_t Key_Read(void);

#endif /* KEY__ */

/******************* (C) COPYRIGHT DUYNV  **********************END OF FILE****/
