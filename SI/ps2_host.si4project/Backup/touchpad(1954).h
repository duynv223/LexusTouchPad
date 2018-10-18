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
#ifndef TOUCH_PAD__
#define TOUCH_PAD__

/******************************************************************************
Includes
******************************************************************************/
#include "stm32f4xx_hal.h"
#include "stdbool.h"


/******************************************************************************
Definitions
******************************************************************************/

typedef struct {
	bool Finger;
	bool Gesture;
	bool Righ;
	bool Left;
	uint16_t XPos;
	uint16_t YPos;
	uint8_t ZPressure;
}Touchpad_MotionPackageTypedef;

/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/
void Touchpad_Init(void);
void Touchpad_RegistMotionPkgReportHandler(void (*f)(Touchpad_MotionPackageTypedef pkg));



#endif /* TOUCH_PAD__ */

/******************* (C) COPYRIGHT DUYNV  **********************END OF FILE****/
