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
#ifndef MOUSE__
#define MOUSE__

/******************************************************************************
Includes
******************************************************************************/
#include "stm32f4xx_hal.h"

/******************************************************************************
Definitions
******************************************************************************/
typedef enum{
	MOUSE_DIR_NONE,
	MOUSE_DIR_R2L,
	MOUSE_DIR_L2R
}MouseDirection_Typedef;
	
typedef struct{
	uint8_t key;
	uint8_t finger;
	MouseDirection_Typedef dirX;
	MouseDirection_Typedef dirY;
	uint8_t	deltaX;
	uint8_t	deltaY;
	
}MouseReport_Typedef;

/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/
void Mouse_Init(void);
void Mouse_RegistReportHandler(void (*f)(MouseReport_Typedef report));


#endif /* MOUSE__ */

/******************* (C) COPYRIGHT DUYNV  **********************END OF FILE****/
