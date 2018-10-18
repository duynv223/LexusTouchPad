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
#ifndef __NameOfFile
#define __NameOfFile

/******************************************************************************
Includes
******************************************************************************/

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
}CmdHistoryHandle_t;

/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/
CmdHistoryHandle_t* CmdHistory_Init(CmdHistoryHandle_t *inst);
void CmdHistory_Add(CmdHistoryHandle_t *inst, const char* cmd);
void CmdHistory_Clr(CmdHistoryHandle_t *inst);
void CmdHistory_BrownInit(CmdHistoryHandle_t *inst);
char* CmdHistory_BrownNext(CmdHistoryHandle_t *inst);
char* CmdHistory_BrownPrev(CmdHistoryHandle_t *inst);


#endif /* __NameOfFile */

/******************* (C) COPYRIGHT DUYNV  **********************END OF FILE****/
