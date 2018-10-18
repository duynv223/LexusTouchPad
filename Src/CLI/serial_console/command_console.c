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
#include "console_ctrl.h"
#include "string.h"
#include "FreeRTOS_CLI.h"

/******************************************************************************
Private Definitions
******************************************************************************/


/* Dimentions a buffer to be used by the UART driver, if the UART driver uses a
buffer at all. */
#define cmdQUEUE_LENGTH			500

/* DEL acts as a backspace. */
#define cmdASCII_DEL		( 0x7F )

typedef struct{
	CONSOLE_PROCESS_EVENT event;
	char* data;
}CONSOLE_MSG;

/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/

/******************************************************************************
Private global variables
******************************************************************************/
static serial_comid eCsComId;
static osMessageQId consoleMgmntEvent;

/* Const messages output by the command console. */
static const char * const pcWelcomeMessage = "FreeRTOS command server.\r\nType Help to view a list of registered commands.\r\n\r\n>>";
static const char * const pcEndOfOutputMessage = "\r\n>>";
static const char * const pcNewLine = "\r\n";

/******************************************************************************
Private function prototypes
******************************************************************************/
static void ConsoleManagerThread(void const *argument);
static void CommandProcessThread(void const *argument);


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
	osThreadDef_t threadDef_1;
	osThreadDef_t threadDef_2;
	
	/* Assign com port id*/
	eCsComId = com;
	
	/* Creat console manager thread */
	threadDef_1.name = "ConsoleMgmnt";
	threadDef_1.pthread = ConsoleManagerThread;
	threadDef_1.tpriority = priority;
	threadDef_1.stacksize = stacksz;
	osThreadCreate(&threadDef_1, NULL);

	/* Creat command process thread */
	threadDef_2.name = "CommandProcess";
	threadDef_2.pthread = CommandProcessThread;
	threadDef_2.tpriority = priority;
	threadDef_2.stacksize = stacksz;
	osThreadCreate(&threadDef_2, NULL);
	
	/* Creat a massage box to comunicate between console-thread 
	   and command-process thread
	*/
	osMessageQDef(osqueue, 5, uint32_t);
	consoleMgmntEvent = osMessageCreate(osMessageQ(osqueue), NULL);
	
}

/*----------------------------------------------------------------------------*\
 * COM SERIAL DRIVER FOR CONSOLE CONTROLLER
\*----------------------------------------------------------------------------*/

uint16_t SerialConsole_Write(const char *buff, uint16_t len)
{
	return serial_write(eCsComId, buff, len);
}

uint16_t SerialConsole_Read(char *buff, uint16_t len)
{
	return serial_read(eCsComId, buff, len);
}

CONSOLE_SERIAL_DRV serialComDrv = 
{
	SerialConsole_Read,
	SerialConsole_Write
};


/*----------------------------------------------------------------------------*\
 * COMMAND CONSOLE THREAD
\*----------------------------------------------------------------------------*/
static void ConsoleManagerThread(void const *argument)
{
	CONSOLE_PROCESS_EVENT consoleEvent;
	CONSOLE_MSG msg;
	(void)argument;
	
	/* Open com port */
	serial_open(eCsComId, 115200);

	/* Initial console */
	Console_Init();

	/* Register serial driver */
	Console_RegisterDriver(&serialComDrv);
	
	/* Send the welcome message. */
	Console_Write(pcWelcomeMessage, strlen( pcWelcomeMessage ));
	
	for(;;)
	{
		/* Call console process rountine every 10mS */
		osDelay(10);
		consoleEvent = Console_Process();
		if(consoleEvent == CONSOLE_EVENT_NEW_CMD)
		{
			msg.event = CONSOLE_EVENT_NEW_CMD;
			msg.data = Console_GetCmd();
			osMessagePut(consoleMgmntEvent, (uint32_t)&msg, 0);
		}
	}
}



static void CommandProcessThread(void const *argument)
{
	char *pcOutputString;
	BaseType_t xReturned;
	const char *cmd;
	osEvent event;
	CONSOLE_MSG *msg;
	/* Obtain the address of the output buffer */
	pcOutputString = FreeRTOS_CLIGetOutputBuffer();
	while(1)
	{
		event = osMessageGet(consoleMgmntEvent, osWaitForever);
		if(event.status == osEventMessage)
		{
			msg = event.value.p;
			if(msg->event == CONSOLE_EVENT_NEW_CMD)
			{

				cmd = msg->data;
				if(strlen(cmd ) >0)
				{
					Console_Write(pcNewLine, ( unsigned short ) strlen( pcNewLine ) );
					/* Handle command */
					do
					{
						/* Get the next output string from the command interpreter. */
						xReturned = FreeRTOS_CLIProcessCommand(cmd , pcOutputString, configCOMMAND_INT_MAX_OUTPUT_SIZE );

						/* Write the generated string to the UART. */
						Console_Write(pcOutputString, ( unsigned short ) strlen( pcOutputString ) );

					} while( xReturned != pdFALSE );
				}
				Console_ClrCmd();
				Console_Write(pcEndOfOutputMessage, ( unsigned short ) strlen( pcEndOfOutputMessage ) );
			}
		}
	}

}






/******************* (C) COPYRIGHT DUYNV  **********************END OF FILE****/
