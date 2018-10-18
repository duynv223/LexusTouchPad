/**
  ******************************************************************************
  * @file    : console_ctrl.c
  * @author  : duynv
  * @version : v0.9
  * @date    : 25/10/2017
  * @brief   :
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/******************************************************************************
Includes
******************************************************************************/
#include "cmd_history.h"
#include "console_ctrl.h"
#include "string.h"

/******************************************************************************
Private Definitions
******************************************************************************/
const char LEFT_ARROW_KEY[] = {0x1B, 0x5B, 0x44};
const char RIGHT_ARROW_KEY[] = {0x1B, 0x5B, 0x43};

typedef enum{
	MOVE_DIR_LEFT,
	MOVE_DIR_RIGHT
}MOVE_DIR;

typedef enum{
	HIS_BR_DIR_PREV,
	HIS_BR_DIR_NEXT
}HIS_BR_DIR;
	
/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/

/******************************************************************************
Private global variables
******************************************************************************/
static char cmdLineBuff[ cmdMAX_INPUT_SIZE ] = {0};
static uint8_t cmdLineLen = 0;
static uint8_t cmdLineCurIndex = 0;
static CONSOLE_SERIAL_DRV *serialDriver;
static CMD_HISTORY *cmdHistory;


/******************************************************************************
Private function prototypes
******************************************************************************/
void Console_CursorMove(MOVE_DIR MOVE_DIR, uint8_t nStep);
void Console_CmdLineInsert(char chr);
void Console_CmdLineDelete(void);
void Console_ReloadHistory(HIS_BR_DIR dir);


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
void Console_Init(void)
{
	cmdHistory = CmdHistory_Creat();
	CmdHistory_BrownReset(cmdHistory);
}

void Console_RegisterDriver(CONSOLE_SERIAL_DRV *driver)
{
	serialDriver = driver;
}

void Console_Write(const char* str, uint16_t len)
{
	serialDriver->write(str, len);
}

char *Console_GetCmd(void)
{
	return cmdLineBuff;
}

void Console_ClrCmd(void)
{
	memset(cmdLineBuff, 0, cmdMAX_INPUT_SIZE);
	cmdLineCurIndex = 0;
	cmdLineLen = 0;
}

CONSOLE_PROCESS_EVENT Console_Process(void)
{
	uint8_t ret = 0;
	static uint8_t isCompleteKey = 1U;
	static uint8_t keyByteIndex = 0;
	signed char chr;
	/* Get char from serial -------------------*/
	if(serialDriver->read((char *)&chr, 1) == 1)
	{
		if(chr == 0x1B)
		{
			isCompleteKey = 0U;
			keyByteIndex = 1U;
		}
		else  if(isCompleteKey == 0U)
		{
			if(keyByteIndex == 1)
				keyByteIndex++;
			else if(keyByteIndex == 2)
			{
				chr = -chr;
				isCompleteKey = 1U;
			}
			else
			{
				/* Do nothing*/
			}
		}
		
		if(isCompleteKey != 0U)
		{
			/* Enter Key ------------------------------*/
			if(chr == '\n'|| chr == '\r')
			{
				/* if command is not emty, save to command history list */
				if(cmdLineLen > 0)
					{
						CmdHistory_Add(cmdHistory, cmdLineBuff);
						CmdHistory_BrownReset(cmdHistory);
					}
				ret = 1;
			}
			if(chr == '\b' )
			{
				Console_CmdLineDelete();
			}
			/* Arrow Left Key -------------------------*/
			else if(chr == -0x44)
			{
				if(cmdLineCurIndex > 0)
				{
					Console_CursorMove(MOVE_DIR_LEFT, 1);
					cmdLineCurIndex--;
				}
			}
			/* Arrow Right Key ------------------------*/
			else if(chr == -0x43)
			{
				if(cmdLineCurIndex < cmdLineLen)
				{
					Console_CursorMove(MOVE_DIR_RIGHT, 1);
					cmdLineCurIndex++;
				}
			}
			/* Arrow Up Key ---------------------------*/
			else if(chr == -0x41)
			{
				Console_ReloadHistory(HIS_BR_DIR_PREV);
			}
			/* Arrow Down Key -------------------------*/
			else if(chr == -0x42)
			{
				Console_ReloadHistory(HIS_BR_DIR_NEXT);
			}
			else
			{
				/* Command character -----------------*/
				if(( chr >= ' ' ) && ( chr <= '~' ))
				{
					if(cmdLineLen <cmdMAX_INPUT_SIZE)
					{
						Console_CmdLineInsert(chr);
					}
				}
			}
		}
	}
	
	return ret;
}

/*-------------------------------------------------------*/
void Console_CursorMove(MOVE_DIR MOVE_DIR, uint8_t nStep)
{
	const char *pKeyChars;
	uint8_t nBytes = sizeof(LEFT_ARROW_KEY);
	
	if(MOVE_DIR == MOVE_DIR_LEFT)
		pKeyChars = LEFT_ARROW_KEY;
	else
		pKeyChars = RIGHT_ARROW_KEY;
	while(nStep--)
		serialDriver->write(pKeyChars, nBytes);
}

/*-------------------------------------------------------*/
void Console_CmdLineInsert(char chr)
{
	uint8_t bytesToMove;
	if(cmdLineLen < cmdMAX_INPUT_SIZE)
	{
		bytesToMove = cmdLineLen - cmdLineCurIndex;
		/* Shift all characters from index to left */
		memmove(&cmdLineBuff[cmdLineCurIndex + 1], &cmdLineBuff[cmdLineCurIndex], bytesToMove);

		/* Store new character at current-index and increace current-index */
		cmdLineBuff[cmdLineCurIndex] = chr;
		cmdLineLen++;
		cmdLineCurIndex++;

		/* Clear end character */
		cmdLineBuff[cmdLineLen] = '\0';

		/* Sync with console */
		/* -- send sub-string from current cursor */
		serialDriver->write(&cmdLineBuff[cmdLineCurIndex - 1], cmdLineLen - cmdLineCurIndex + 1);
		/* -- move cursor back */
		Console_CursorMove(MOVE_DIR_LEFT, cmdLineLen - cmdLineCurIndex);
	}

}

/*-------------------------------------------------------*/
void Console_CmdLineDelete(void)
{
	uint8_t bytesToMove;
	if(cmdLineCurIndex > 0)
	{
		bytesToMove = cmdLineLen - cmdLineCurIndex;
		/* Shift all characters from index to right */
		memmove(&cmdLineBuff[cmdLineCurIndex - 1], &cmdLineBuff[cmdLineCurIndex], bytesToMove);

		/* Increace index and len */
		cmdLineCurIndex--;
		cmdLineLen--;	
		
		/* Remove last character */
		cmdLineBuff[cmdLineLen] = '\0';
		
		/* Sync with console */
		/* -- send sub-string from previous cursor */
		Console_CursorMove(MOVE_DIR_LEFT, 1);
		serialDriver->write(&cmdLineBuff[cmdLineCurIndex], cmdLineLen - cmdLineCurIndex);
		serialDriver->write(" ", 1);
		/* -- move cursor back */
		Console_CursorMove(MOVE_DIR_LEFT, cmdLineLen - cmdLineCurIndex + 1);
	}
}

/*-------------------------------------------------------*/
void Console_CmdLineClr(void)
{
	
}

/*-------------------------------------------------------*/
void Console_ReloadHistory(HIS_BR_DIR dir)
{
	char *cmd;
	if(dir == HIS_BR_DIR_PREV)	
		cmd = CmdHistory_BrownPrev(cmdHistory);
	else
		cmd = CmdHistory_BrownNext(cmdHistory);
	
	if(cmd != NULL)
	{
		size_t len = strlen(cmd);
		//assert_param(len <= cmdMAX_INPUT_SIZE);
		
		/* Update to console */
		/* -- Move to the end of current command */
		Console_CursorMove(MOVE_DIR_RIGHT, cmdLineLen - cmdLineCurIndex);
		/* -- Delete by send backspace */
		while(cmdLineLen--)
		{
			serialDriver->write("\b \b", 3);
		}
		/* -- Write new command string*/
		serialDriver->write(cmd, len);

		/* Update to cmdLineBuff */
		strcpy(cmdLineBuff, cmd);
		cmdLineCurIndex = len;
		cmdLineLen = len;
	}
	
	
}

/******************* (C) COPYRIGHT DUYNV  **********************END OF FILE****/
