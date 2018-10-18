/**
  ******************************************************************************
  * @file    : cmd_history.h
  * @author  : duynv
  * @version : v0.9
  * @date    : 24/10/2017
  * @brief   :
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CMD_HISTORY__
#define __CMD_HISTORY__

/******************************************************************************
Includes
******************************************************************************/
#include "cmsis_os.h"

/******************************************************************************
Definitions
******************************************************************************/
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

typedef struct CmdHistoryHandle
{
	struct list historyList;
	struct elt *pCurrentBrowseElt;	
}CMD_HISTORY;

/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/
CMD_HISTORY* CmdHistory_Creat(void);
void CmdHistory_Add(CMD_HISTORY *handle, const char* cmd);
void CmdHistory_Clr(CMD_HISTORY *handle);
void CmdHistory_BrownInit(CMD_HISTORY *handle);
char* CmdHistory_BrownNext(CMD_HISTORY *handle);
char* CmdHistory_BrownPrev(CMD_HISTORY *handle);
char CmdHistory_BrownReset(CMD_HISTORY *handle);


#endif /* __CMD_HISTORY__ */

/******************* (C) COPYRIGHT DUYNV  **********************END OF FILE****/
