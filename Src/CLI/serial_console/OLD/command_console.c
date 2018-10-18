/**
  ******************************************************************************
  * @file    : command_console.c
  * @author  : duynv4	
  * @version : v0.9
  * @date    : 15/10/2017
  * @brief   : 
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/******************************************************************************
Includes
******************************************************************************/
#include "command_console.h"
#include "string.h"
#include "FreeRTOS_CLI.h"

/******************************************************************************
Private Definitions
******************************************************************************/
/* Dimensions the buffer into which input characters are placed. */
#define cmdMAX_INPUT_SIZE		100

/* Dimentions a buffer to be used by the UART driver, if the UART driver uses a
buffer at all. */
#define cmdQUEUE_LENGTH			500

/* DEL acts as a backspace. */
#define cmdASCII_DEL		( 0x7F )

/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/

/******************************************************************************
Private global variables
******************************************************************************/
static serial_comid eCsComId;
/* Const messages output by the command console. */
static const char * const pcWelcomeMessage = "FreeRTOS command server.\r\nType Help to view a list of registered commands.\r\n\r\n>>";
static const char * const pcEndOfOutputMessage = "\r\n>>";
static const char * const pcNewLine = "\r\n";

/******************************************************************************
Private function prototypes
******************************************************************************/
static void commandConsoleThread(void const *argument);

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

void commandConsoleStart(serial_comid com, uint16_t stacksz, osPriority priority)
{
	osThreadDef_t threadDef;
	/* Assign com port id*/
	eCsComId = com;
	
	/* Creat new thread */
	threadDef.name = "CLI";
	threadDef.pthread = commandConsoleThread;
	threadDef.tpriority = priority;
	threadDef.stacksize = stacksz;
	osThreadCreate(&threadDef, NULL);
}

/*----------------------------------------------------------------------------*\
 * COMMAND HISTORY LIST 
\*----------------------------------------------------------------------------*/
/*
	- Fifo fashion: when the list is full, the oldest is dequeue, the new one is 
					enqueue
	- Action:
		+ add: 
			+ check this command is new? if true, move it to top
			+ add at the top of list
		+ browse (by use up and down keys):
			+ update console following current cmd in list
		+ recall:
			+ move current cmd in list to top
	--> fifo action:
		+ move a element to top
		+ browse next and previous
	- Design: using linked list? or array?
*/
#define cmdMAX_HISTORY_SIZE	5U
#define cmd_malloc				pvPortMalloc
#define cmd_free				vPortFree

struct elt{
	struct elt *next;
	struct elt *prev;
	char *content;
};

struct list{
	uint8_t count;
	struct elt *head;
	struct elt *tail;
};

struct list historyList;
struct elt *pCurrentBrowseElt;

static void prvHistoryCmds_Init(void)
{
	historyList.count = 0;
	historyList.head = NULL;
	historyList.tail = NULL;
}

static void prvHistoryCmds_Add(const char *cmd)
{
	struct elt *pNewCmd;
	struct elt *pTail;
	char *pcCmd;
	size_t len = strlen(cmd);
	/* Check if new cmd is like the last -------------------------*/
	if(historyList.count != 0)
	{
		if(strcmp(historyList.head->content, cmd) == 0)
			return;
	}
	/* Creat new element -----------------------------------------*/
	assert_param(len <= cmdMAX_HISTORY_SIZE);
	pNewCmd = (struct elt *)cmd_malloc(sizeof(struct elt));
	pcCmd = (char *)cmd_malloc(len + 1);
	strcpy(pcCmd, cmd);
	pNewCmd->content = pcCmd;
	pNewCmd->prev = NULL;

	/* Add to list ------------------------------------------------*/
	/* pre-emty list */
	if(historyList.count == 0)
	{
		historyList.head = pNewCmd;
		historyList.tail = pNewCmd;
		pNewCmd->next = NULL;
	}
	/* exist element */
	else
	{
		historyList.head->prev = pNewCmd;
		pNewCmd->next = historyList.head;
		historyList.head = pNewCmd;
	}
	historyList.count++;
	
	/* read max size remove the last */
	if(historyList.count > cmdMAX_HISTORY_SIZE)
	{
		cmd_free(historyList.tail->content);
		pTail = historyList.tail;
		historyList.tail =  historyList.tail->prev;
		cmd_free(pTail);
	}
}

static void prvHistoryCmds_BrownReset(void)
{
	pCurrentBrowseElt = NULL;
}

static char* prvHistoryCmds_BrownPrevious(void)
{
	char* ret = NULL;
	if(pCurrentBrowseElt == NULL)
	{
		pCurrentBrowseElt = historyList.head;
	}
	else
	{
		if(pCurrentBrowseElt->next != NULL)
			pCurrentBrowseElt = pCurrentBrowseElt->next;
	}
	
	if(pCurrentBrowseElt != NULL)
	{
		ret =  pCurrentBrowseElt->content;
	}

	return ret;
}

static char* prvHistoryCmds_BrownNext(void)
{
	char *ret = NULL;
	if(pCurrentBrowseElt != NULL)
	{
		if(pCurrentBrowseElt->prev != NULL)
		{
			pCurrentBrowseElt = pCurrentBrowseElt->prev;
			ret = pCurrentBrowseElt->content;
		}
	}
	return ret;
}



/*----------------------------------------------------------------------------*\
 * COMMAND COSOLE CONTROL 
\*----------------------------------------------------------------------------*/

static char cCmdStr[ cmdMAX_INPUT_SIZE ] = {0};
static uint8_t ucCmdStrLen = 0;
static uint8_t ucCmdCurrInd = 0;

static void prvConsole_ClearCommand()
{
	memset(cCmdStr, 0, cmdMAX_INPUT_SIZE);
	ucCmdStrLen = 0;
	ucCmdCurrInd = 0;
}

static char* prvConsole_GetCommand()
{
	return cCmdStr;
}

static void prvConsole_MoveCursor(uint8_t isLeft, uint8_t nStep)
{
	const char leftArrow[3] = {0x1B, 0x5B, 0x44};
	const char rightArrow[3] = {0x1B, 0x5B, 0x43};
	const char *arrow;
	if(isLeft != 0)
	{
		arrow = leftArrow;
	}
	else
	{
		arrow = rightArrow;
	}
	
	while(nStep--)
	{
		serial_write(eCsComId, arrow, 3);
	}
}

static void prvConsole_Add(char chr)
{
	uint8_t bytesToMove;
	if(ucCmdStrLen < cmdMAX_INPUT_SIZE)
	{
		bytesToMove = ucCmdStrLen - ucCmdCurrInd;
		/* Shift all characters from index to left */
		memmove(&cCmdStr[ucCmdCurrInd + 1], &cCmdStr[ucCmdCurrInd], bytesToMove);

		/* Store new character at current-index and increace current-index */
		cCmdStr[ucCmdCurrInd] = chr;
		ucCmdStrLen++;
		ucCmdCurrInd++;

		/* Clear end character */
		cCmdStr[ucCmdStrLen] = '\0';

		/* Sync with console */
		/* -- send sub-string from current cursor */
		serial_write(eCsComId, &cCmdStr[ucCmdCurrInd - 1], ucCmdStrLen - ucCmdCurrInd + 1);
		/* -- move cursor back */
		prvConsole_MoveCursor(1, ucCmdStrLen - ucCmdCurrInd);
	}

}

static void prvConsole_Delete(void)
{
	uint8_t bytesToMove;
	if(ucCmdCurrInd > 0)
	{
		bytesToMove = ucCmdStrLen - ucCmdCurrInd;
		/* Shift all characters from index to right */
		memmove(&cCmdStr[ucCmdCurrInd - 1], &cCmdStr[ucCmdCurrInd], bytesToMove);

		/* Increace index and len */
		ucCmdCurrInd--;
		ucCmdStrLen--;	
		
		/* Remove last character */
		cCmdStr[ucCmdStrLen] = '\0';
		
		/* Sync with console */
		/* -- send sub-string from previous cursor */
		prvConsole_MoveCursor(1, 1);
		serial_write(eCsComId, &cCmdStr[ucCmdCurrInd], ucCmdStrLen - ucCmdCurrInd);
		serial_write(eCsComId, " ", 1);
		/* -- move cursor back */
		prvConsole_MoveCursor(1, ucCmdStrLen - ucCmdCurrInd + 1);
	}
}


static void prvConsole_Move(uint8_t isLeft)
{
	if(isLeft != 0U)
	{
		if(ucCmdCurrInd > 0)
		{
			prvConsole_MoveCursor(1, 1);
			ucCmdCurrInd--;
		}
	}
	else
	{
		if(ucCmdCurrInd < ucCmdStrLen)
		{
			prvConsole_MoveCursor(0, 1);
			ucCmdCurrInd++;
		}

	}
}

static void prvConsole_History(uint8_t isPrev)
{
	char *cmd;
	if(isPrev != 0)	
		cmd = prvHistoryCmds_BrownPrevious();
	else
		cmd = prvHistoryCmds_BrownNext();
	
	if(cmd != NULL)
	{
		size_t len = strlen(cmd);
		assert_param(len <= cmdMAX_INPUT_SIZE);
		
		/* Update to console */
		/* -- Move to the end of current command */
		prvConsole_MoveCursor(0, ucCmdStrLen - ucCmdCurrInd);
		/* -- Delete by send backspace */
		while(ucCmdStrLen--)
		{
			serial_write(eCsComId, "\b \b", 3);
		}
		/* -- Write new command string*/
		serial_write(eCsComId, cmd, len);

		/* Update to cCmdStr */
		strcpy(cCmdStr, cmd);
		ucCmdCurrInd = len;
		ucCmdStrLen = len;
	}
}

static uint8_t prvConsole_HandleKey(signed char chr)
{
	uint8_t ret = 0;
	/* Enter Key ------------------------------*/
	if(chr == '\n'|| chr == '\r')
	{
		/* if command is not emty, save to command history list */
		if(ucCmdStrLen > 0)
			{
				prvHistoryCmds_Add(cCmdStr);
				prvHistoryCmds_BrownReset();
			}
		ret = 1;
	}
	if(chr == '\b' )
	{
		prvConsole_Delete();
	}
	/* Arrow Left Key -------------------------*/
	else if(chr == -0x44)
	{
		prvConsole_Move(1);
	}
	/* Arrow Right Key ------------------------*/
	else if(chr == -0x43)
	{
		prvConsole_Move(0);
	}
	/* Arrow Up Key ---------------------------*/
	else if(chr == -0x41)
	{
		prvConsole_History(1);
	}
	/* Arrow Down Key -------------------------*/
	else if(chr == -0x42)
	{
		prvConsole_History(0);
	}
	else
	{
		/* Command character -----------------*/
		if(( chr >= ' ' ) && ( chr <= '~' ))
		{
			if(ucCmdStrLen <cmdMAX_INPUT_SIZE)
			{
				prvConsole_Add(chr);
			}
		}
	}

	return ret;
}


/*----------------------------------------------------------------------------*\
 * COMMAND CONSOLE THREAD
\*----------------------------------------------------------------------------*/


static void commandConsoleThread(void const *argument)
{
	char *pcOutputString;
	signed char cRcvChar;
	BaseType_t xReturned;
	
	(void)argument;
	
	/* Open com port */
	serial_open(eCsComId, 115200);
	
	/* Obtain the address of the output buffer */
	pcOutputString = FreeRTOS_CLIGetOutputBuffer();
	
	/* Send the welcome message. */
	serial_write(eCsComId, pcWelcomeMessage, strlen( pcWelcomeMessage ));

	/* Init command history list */
	prvHistoryCmds_Init();
	
	for(;;)
	{
		/* Get the input single character from serial -------------------------------------------*/
		/* Polling serial every 10mS to get new character */
		while(serial_read(eCsComId, (char *)&cRcvChar, 1) == 0)
		{
			osDelay(10);
		}

		/* Detect arrow keys 
		   Arrow key in ascii: 1Bh-5Bh-xx (U: 41h, D: 42h, R: 43h, L: 44h)
		*/
		if(cRcvChar == 0x1B)
		{
				/* Get next 2 byte */
				while(serial_read(eCsComId, (char *)&cRcvChar, 1) == 0){osDelay(10);}
				if(cRcvChar == 0x5B)
				{
					while(serial_read(eCsComId, (char *)&cRcvChar, 1) == 0){osDelay(10);}
					cRcvChar = -cRcvChar;
				}
				else
				{
					/* ESC key is pressed */
				}
		}

		/* Hand received key --------------------------------------------------------------------*/
		if(prvConsole_HandleKey(cRcvChar) == 1)
		{
			if(strlen(prvConsole_GetCommand()) >0)
			{
				serial_write(eCsComId, pcNewLine, ( unsigned short ) strlen( pcNewLine ) );
				/* Handle command */
				do
				{
					/* Get the next output string from the command interpreter. */
					xReturned = FreeRTOS_CLIProcessCommand( (const char *)prvConsole_GetCommand(), pcOutputString, configCOMMAND_INT_MAX_OUTPUT_SIZE );

					/* Write the generated string to the UART. */
					serial_write(eCsComId, pcOutputString, ( unsigned short ) strlen( pcOutputString ) );

				} while( xReturned != pdFALSE );
			}
			prvConsole_ClearCommand();
			serial_write(eCsComId, pcEndOfOutputMessage, ( unsigned short ) strlen( pcEndOfOutputMessage ) );
		}
	}
}





/******************* (C) COPYRIGHT DUYNV  **********************END OF FILE****/
