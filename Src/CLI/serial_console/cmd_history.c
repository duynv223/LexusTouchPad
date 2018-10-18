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

/******************************************************************************
Includes
******************************************************************************/
#include "cmd_history.h"

/******************************************************************************
Private Definitions
******************************************************************************/
#define CmdHistory_Malloc				pvPortMalloc
#define CmdHistory_Free					vPortFree
#define cmdMAX_HISTORY_SIZE			5

/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/

/******************************************************************************
Private global variables
******************************************************************************/

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
CMD_HISTORY* CmdHistory_Creat(void)
{
	CMD_HISTORY* pCmdHist;
	pCmdHist = (CMD_HISTORY*)CmdHistory_Malloc(sizeof(CMD_HISTORY));
	pCmdHist->historyList.head = NULL;
	pCmdHist->historyList.tail = NULL;
	pCmdHist->historyList.count = 0;
	return pCmdHist;
}

void CmdHistory_Add(CMD_HISTORY *handle, const char* cmd)
{
	struct elt *pNewCmd;
	struct elt *pTail;
	char *pcCmd;
	size_t len = strlen(cmd);
	/* Check if new cmd is like the last -------------------------*/
	if(handle->historyList.count != 0)
	{
		if(strcmp(handle->historyList.head->content, cmd) == 0)
			return;
	}
	/* Creat new element -----------------------------------------*/
	//assert_param(len <= cmdMAX_HISTORY_SIZE);
	pNewCmd = (struct elt *)CmdHistory_Malloc(sizeof(struct elt));
	pcCmd = (char *)CmdHistory_Malloc(len + 1);
	strcpy(pcCmd, cmd);
	pNewCmd->content = pcCmd;
	pNewCmd->prev = NULL;

	/* Add to list ------------------------------------------------*/
	/* pre-emty list */
	if(handle->historyList.count == 0)
	{
		handle->historyList.head = pNewCmd;
		handle->historyList.tail = pNewCmd;
		pNewCmd->next = NULL;
	}
	/* exist element */
	else
	{
		handle->historyList.head->prev = pNewCmd;
		pNewCmd->next = handle->historyList.head;
		handle->historyList.head = pNewCmd;
	}
	handle->historyList.count++;
	
	/* read max size remove the last */
	if(handle->historyList.count > cmdMAX_HISTORY_SIZE)
	{
		CmdHistory_Free(handle->historyList.tail->content);
		pTail = handle->historyList.tail;
		handle->historyList.tail =  handle->historyList.tail->prev;
		CmdHistory_Free(pTail);
	}
}

char CmdHistory_BrownReset(CMD_HISTORY *handle)
{
	handle->pCurrentBrowseElt = NULL;
}


char* CmdHistory_BrownPrev(CMD_HISTORY *handle)
{
	char* ret = NULL;
	if(handle->pCurrentBrowseElt == NULL)
	{
		handle->pCurrentBrowseElt = handle->historyList.head;
	}
	else
	{
		if(handle->pCurrentBrowseElt->next != NULL)
			handle->pCurrentBrowseElt = handle->pCurrentBrowseElt->next;
	}
	
	if(handle->pCurrentBrowseElt != NULL)
	{
		ret =  handle->pCurrentBrowseElt->content;
	}

	return ret;
}

char* CmdHistory_BrownNext(CMD_HISTORY *handle)
{
	char *ret = NULL;
	if(handle->pCurrentBrowseElt != NULL)
	{
		if(handle->pCurrentBrowseElt->prev != NULL)
		{
			handle->pCurrentBrowseElt = handle->pCurrentBrowseElt->prev;
			ret = handle->pCurrentBrowseElt->content;
		}
	}
	return ret;
}


/******************* (C) COPYRIGHT DUYNV  **********************END OF FILE****/
