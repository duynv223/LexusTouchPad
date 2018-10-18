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
CmdHistoryHandle_t* CmdHistory_Creat(CmdHistoryHandle_t *inst)
{
	CmdHistoryHandle_t* pCmdHist;
	pCmdHist = (CmdHistoryHandle_t*)CmdHistory_Malloc(sizeof(CmdHistoryHandle_t));
	pCmdHist->historyList.head = NULL;
	pCmdHist->historyList.tail = NULL;
	pCmdHist->historyList.count = 0;
	return pCmdHist;
}

void CmdHistory_Add(CmdHistoryHandle_t *inst, const char* cmd)
{
	struct elt *pNewCmd;
	struct elt *pTail;
	char *pcCmd;
	size_t len = strlen(cmd);
	/* Check if new cmd is like the last -------------------------*/
	if(inst->historyList.count != 0)
	{
		if(strcmp(inst->historyList.head->content, cmd) == 0)
			return;
	}
	/* Creat new element -----------------------------------------*/
	assert_param(len <= cmdMAX_HISTORY_SIZE);
	pNewCmd = (struct elt *)CmdHistory_Malloc(sizeof(struct elt));
	pcCmd = (char *)CmdHistory_Malloc(len + 1);
	strcpy(pcCmd, cmd);
	pNewCmd->content = pcCmd;
	pNewCmd->prev = NULL;

	/* Add to list ------------------------------------------------*/
	/* pre-emty list */
	if(inst->historyList.count == 0)
	{
		inst->historyList.head = pNewCmd;
		inst->historyList.tail = pNewCmd;
		pNewCmd->next = NULL;
	}
	/* exist element */
	else
	{
		inst->historyList.head->prev = pNewCmd;
		pNewCmd->next = inst->historyList.head;
		inst->historyList.head = pNewCmd;
	}
	inst->historyList.count++;
	
	/* read max size remove the last */
	if(inst->historyList.count > cmdMAX_HISTORY_SIZE)
	{
		CmdHistory_Free(inst->historyList.tail->content);
		pTail = inst->historyList.tail;
		inst->historyList.tail =  inst->historyList.tail->prev;
		CmdHistory_Free(pTail);
	}
}

char* CmdHistory_BrownPrev(CmdHistoryHandle_t *inst)
{
	char* ret = NULL;
	if(inst->pCurrentBrowseElt == NULL)
	{
		inst->pCurrentBrowseElt = inst->historyList.head;
	}
	else
	{
		if(inst->pCurrentBrowseElt->next != NULL)
			inst->pCurrentBrowseElt = inst->pCurrentBrowseElt->next;
	}
	
	if(inst->pCurrentBrowseElt != NULL)
	{
		ret =  inst->pCurrentBrowseElt->content;
	}

	return ret;
}

char* CmdHistory_BrownNext(CmdHistoryHandle_t *inst)
{
	char *ret = NULL;
	if(inst->pCurrentBrowseElt != NULL)
	{
		if(inst->pCurrentBrowseElt->prev != NULL)
		{
			inst->pCurrentBrowseElt = inst->pCurrentBrowseElt->prev;
			ret = inst->pCurrentBrowseElt->content;
		}
	}
	return ret;
}


/******************* (C) COPYRIGHT DUYNV  **********************END OF FILE****/
