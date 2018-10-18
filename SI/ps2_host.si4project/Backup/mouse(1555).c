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
#include "mouse.h"
#include "TouchPad.h"

/******************************************************************************
Private Definitions
******************************************************************************/
#define TOUCHPAD_X_MIN		1200
#define TOUCHPAD_Y_MIN		1200
#define TOUCHPAD_X_MAX		5500
#define TOUCHPAD_Y_MAX		4500

typedef enum{
	MOUSE_STATE_IDLE,
	MOUSE_STATE_ON_TOUCH
}MouseState_Typedef;
	
/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/

/******************************************************************************
Private global variables
******************************************************************************/
void (*g_MouseReportHandler)(MouseReport_Typedef report);

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

void MotionPkgReportHandler(Touchpad_MotionPackageTypedef pkg)
{
	/* report with rate 40 package/s T = 25mS */
	static MouseState_Typedef state = MOUSE_STATE_IDLE;
	static uint16_t prePosX, prePosY;
	MouseReport_Typedef report;
	switch(state)
		{
			case MOUSE_STATE_IDLE:
				if(pkg.XPos >= TOUCHPAD_X_MIN && pkg.XPos >= TOUCHPAD_Y_MIN)
					{
						prePosX = pkg.XPos;
						prePosY = pkg.YPos;
						state = MOUSE_STATE_ON_TOUCH;
					}
				break;
			case MOUSE_STATE_ON_TOUCH:
				if(pkg.XPos < TOUCHPAD_X_MIN && pkg.XPos < TOUCHPAD_Y_MIN)
					{
						state = MOUSE_STATE_IDLE;
					}
				else
					{
						if(pkg.XPos == prePosX)
							{
								report.dirX = MOUSE_DIR_NONE;
								report.deltaX = 0;							

							}
						else if(pkg.XPos > prePosX)
							{
								report.dirX = MOUSE_DIR_L2R;
								report.deltaX = pkg.XPos - prePosX;	
							}
						else
							{
								report.dirX = MOUSE_DIR_R2L;
								report.deltaX = prePosX - pkg.XPos;	
							}		
						
						if(pkg.YPos == prePosY)
							{
								report.dirY = MOUSE_DIR_NONE;
								report.deltaY = 0;							

							}
						else if(pkg.YPos > prePosY)
							{
								report.dirY = MOUSE_DIR_L2R;
								report.deltaY = pkg.YPos - prePosY;	
							}
						else
							{
								report.dirY = MOUSE_DIR_R2L;
								report.deltaY = prePosY - pkg.YPos;	
							}	

						g_MouseReportHandler(report);
						prePosX = pkg.XPos;
						prePosY = pkg.YPos;
					}
				break;
			default:
				break;
		}
}

void Mouse_Init(void)
{
	Touchpad_Init();
	Touchpad_RegistMotionPkgReportHandler(MotionPkgReportHandler);
}
void Mouse_RegistReportHandler(void (*f)(MouseReport_Typedef report))
{
	g_MouseReportHandler = f;
}

/******************* (C) COPYRIGHT DUYNV  **********************END OF FILE****/
