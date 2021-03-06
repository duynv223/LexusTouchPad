/**
  ******************************************************************************
 * @file    : command_console.h
  * @author  : duynv4	
  * @version : v0.9
  * @date    : 15/10/2017
  * @brief   :
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMMAND_CONSOLE_H
#define __COMMAND_CONSOLE_H

/******************************************************************************
Includes
******************************************************************************/
#include "serial.h"
#include "cmsis_os.h"

/******************************************************************************
Definitions
******************************************************************************/

/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/
void commandConsoleStart(serial_comid com, uint16_t stacksz, osPriority priority);






#endif /* __COMMAND_CONSOLE_H */

/******************* (C) COPYRIGHT DUYNV  **********************END OF FILE****/
