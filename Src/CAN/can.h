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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef CAN_H_
#define CAN_H_

/******************************************************************************
Includes
******************************************************************************/
#include "stm32f4xx_hal.h"

/******************************************************************************
Definitions
******************************************************************************/
#define CANx_CLK_ENABLE()			__HAL_RCC_CAN1_CLK_ENABLE()
#define CANx_TX_CLK_ENABLE()		__HAL_RCC_GPIOD_CLK_ENABLE()
#define CANx_RX_CLK_ENABLE()		__HAL_RCC_GPIOD_CLK_ENABLE()

#define CANx_TX_PIN					GPIO_PIN_1
#define CANx_TX_PORT				GPIOD
#define CANx_TX_AF					GPIO_AF9_CAN1

#define CANx_RX_PIN					GPIO_PIN_0
#define CANx_RX_PORT				GPIOD
#define CANx_RX_AF					GPIO_AF9_CAN1

#define CANx_RX_IRQn                CAN1_RX0_IRQn
#define CANx_RX_IRQHandler          CAN1_RX0_IRQHandler


/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/
void CAN_Init(void);
void CAN_Transmit(CanTxMsgTypeDef *msg);
void CAN_RegistReceivedMsgHandler(void (*f)(CanRxMsgTypeDef *msg));
void CAN_RxCpltHandler(void);


#endif /* CAN_H_ */

/******************* (C) COPYRIGHT DUYNV  **********************END OF FILE****/
