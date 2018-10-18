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

/******************************************************************************
Includes
******************************************************************************/
#include "key.h"

/******************************************************************************
Private Definitions
******************************************************************************/


/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/

/******************************************************************************
Private global variables
******************************************************************************/

/******************************************************************************
Private function prototypes
******************************************************************************/

/******************************************************************************
Private functions
******************************************************************************/

/*****************************************************************************
* Function Name : 
* Description   : 
* Parameter     : None
* Return        : None
* Attention     : None
******************************************************************************/
void Key_Init(void)
{

	GPIO_InitTypeDef GPIO_InitStruct;
	
	KEY_MAP_CLK_EN();
	GPIO_InitStruct.Pin = KEY_MAP_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(KEY_MAP_PORT, &GPIO_InitStruct);
	
	KEY_MENU_CLK_EN();
	GPIO_InitStruct.Pin = KEY_MENU_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(KEY_MENU_PORT, &GPIO_InitStruct);
	
	KEY_BACK_CLK_EN();
	GPIO_InitStruct.Pin = KEY_BACK_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(KEY_BACK_PORT, &GPIO_InitStruct);
	
	KEY_UP_CLK_EN();
	GPIO_InitStruct.Pin = KEY_UP_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(KEY_UP_PORT, &GPIO_InitStruct);
	
}

uint8_t Key_Read(void)
{
	uint8_t keyCode = 0;

	if(HAL_GPIO_ReadPin(KEY_MAP_PORT, KEY_MAP_PIN) 	== GPIO_PIN_SET)
	{
		keyCode |= BIT_MASK_KEY_MAP;
	}
	if(HAL_GPIO_ReadPin(KEY_MENU_PORT, KEY_MENU_PIN) == GPIO_PIN_SET)
	{
		keyCode |= BIT_MASK_KEY_MENU;
	}
	if(HAL_GPIO_ReadPin(KEY_BACK_PORT, KEY_BACK_PIN) == GPIO_PIN_SET)
	{
		keyCode |= BIT_MASK_KEY_BACK;
	}
	if(HAL_GPIO_ReadPin(KEY_UP_PORT, KEY_UP_PIN) 	== GPIO_PIN_SET)
	{
		keyCode |= BIT_MASK_KEY_UP;
	}
	
	return keyCode;
}

/******************* (C) COPYRIGHT DUYNV  **********************END OF FILE****/
