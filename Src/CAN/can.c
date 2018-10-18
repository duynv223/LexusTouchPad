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
#include "can.h"
#include "string.h"

/******************************************************************************
Private Definitions
******************************************************************************/
CAN_HandleTypeDef g_CanHandle;
void (*gp_CanReceivedMsgCb)(CanRxMsgTypeDef *msg);

/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/

/******************************************************************************
Private global variables
******************************************************************************/

/******************************************************************************
Private function prototypes
******************************************************************************/
//static void CAN_Thread(void const *argument);

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
void CAN_MspInit()
{
  GPIO_InitTypeDef   GPIO_InitStruct;
  
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* CAN1 Periph clock enable */
  CANx_CLK_ENABLE();
  /* Enable GPIO clock ****************************************/
  CANx_TX_CLK_ENABLE();
  CANx_RX_CLK_ENABLE();
  
  /*##-2- Configure peripheral GPIO ##########################################*/ 
  /* CAN1 TX GPIO pin configuration */
  GPIO_InitStruct.Pin = CANx_TX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Alternate =  CANx_TX_AF;
  
  HAL_GPIO_Init(CANx_TX_PORT, &GPIO_InitStruct);
  
  /* CAN1 RX GPIO pin configuration */
  GPIO_InitStruct.Pin = CANx_RX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Alternate =  CANx_RX_AF;
  
  HAL_GPIO_Init(CANx_RX_PORT, &GPIO_InitStruct);
  
  /*##-3- Configure the NVIC #################################################*/
  /* NVIC configuration for CAN1 Reception complete interrupt */
  HAL_NVIC_SetPriority(CANx_RX_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(CANx_RX_IRQn);
}


void CAN_Init(void)
{
	CAN_FilterConfTypeDef         sFilterConfig;
	static CanTxMsgTypeDef        TxMessage;
	static CanRxMsgTypeDef        RxMessage;

	CAN_MspInit();
		
	/*##-1- Configure the CAN peripheral #######################################*/
	g_CanHandle.Instance = CAN1;
	g_CanHandle.pTxMsg = &TxMessage;
	g_CanHandle.pRxMsg = &RxMessage;

	g_CanHandle.Init.TTCM = DISABLE;
	g_CanHandle.Init.ABOM = DISABLE;
	g_CanHandle.Init.AWUM = DISABLE;
	g_CanHandle.Init.NART = DISABLE;
	g_CanHandle.Init.RFLM = DISABLE;
	g_CanHandle.Init.TXFP = DISABLE;
	g_CanHandle.Init.Mode = CAN_MODE_NORMAL;
	g_CanHandle.Init.SJW = CAN_SJW_1TQ;
	g_CanHandle.Init.BS1 = CAN_BS1_12TQ;
	g_CanHandle.Init.BS2 = CAN_BS2_8TQ;
	g_CanHandle.Init.Prescaler = 4;

	if(HAL_CAN_Init(&g_CanHandle) != HAL_OK)
	{
	}

	/*##-2- Configure the CAN Filter ###########################################*/
	sFilterConfig.FilterNumber = 0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = 0x0000;
	sFilterConfig.FilterIdLow = 0x0000;
	sFilterConfig.FilterMaskIdHigh = 0x0000;
	sFilterConfig.FilterMaskIdLow = 0x0000;
	sFilterConfig.FilterFIFOAssignment = 0;
	sFilterConfig.FilterActivation = ENABLE;
	sFilterConfig.BankNumber = 14;

	if(HAL_CAN_ConfigFilter(&g_CanHandle, &sFilterConfig) != HAL_OK)
	{
	}
	  
	/*##-3- Configure Transmission process #####################################*/
	g_CanHandle.pTxMsg->StdId = 0x1F1;
	g_CanHandle.pTxMsg->ExtId = 0x01;
	g_CanHandle.pTxMsg->RTR = CAN_RTR_DATA;
	g_CanHandle.pTxMsg->IDE = CAN_ID_STD;
	g_CanHandle.pTxMsg->DLC = 1;

	/* creat can thread */
	//osThreadDef(CAN_Thread, CAN_Thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);


	HAL_CAN_Receive_IT(&g_CanHandle, CAN_FIFO0);
  
  
}

void CAN_RxCpltHandler(void)
{
	if(gp_CanReceivedMsgCb)
		gp_CanReceivedMsgCb(g_CanHandle.pRxMsg);
		
	/* Receive */
	HAL_CAN_Receive_IT(&g_CanHandle, CAN_FIFO0);

}

void CAN_Transmit(CanTxMsgTypeDef *msg)
{
	memcpy(g_CanHandle.pTxMsg, msg, sizeof(CanTxMsgTypeDef));
	HAL_CAN_Transmit(&g_CanHandle, 10);
}

void CAN_RegistReceivedMsgHandler(void (*f)(CanRxMsgTypeDef *msg))
{
	gp_CanReceivedMsgCb = f;
}


/******************* (C) COPYRIGHT DUYNV  **********************END OF FILE****/
