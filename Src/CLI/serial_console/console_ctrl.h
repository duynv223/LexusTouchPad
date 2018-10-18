/**
  ******************************************************************************
  * @file    : console_ctrl.h
  * @author  : duynv
  * @version : v0.9
  * @date    : 25/10/2017
  * @brief   :
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CONSOLE_CTRL_
#define __CONSOLE_CTRL_

/******************************************************************************
Includes
******************************************************************************/
#include "cmsis_os.h"

/******************************************************************************
Definitions
******************************************************************************/
typedef struct{
	uint16_t (*read)(char *, uint16_t);
	uint16_t (*write)(const char *, uint16_t);
}CONSOLE_SERIAL_DRV;

typedef enum{
	CONSOLE_EVENT_NOTHING,
	CONSOLE_EVENT_NEW_CMD,
	CONSOLE_EVENT_TERMINATE,
}CONSOLE_PROCESS_EVENT;

/* Dimensions the buffer into which input characters are placed. */
#define cmdMAX_INPUT_SIZE		100

/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/
void Console_Init(void);
void Console_RegisterDriver(CONSOLE_SERIAL_DRV *driver);
void Console_Write(const char* str, uint16_t len);
CONSOLE_PROCESS_EVENT Console_Process(void);
char *Console_GetCmd(void);
void Console_ClrCmd(void);



#endif /* __CONSOLE_CTRL_ */

/******************* (C) COPYRIGHT DUYNV  **********************END OF FILE****/
