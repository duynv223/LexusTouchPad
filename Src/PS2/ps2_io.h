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
#ifndef PS2_IO__
#define PS2_IO__

/******************************************************************************
Includes
******************************************************************************/
#include "stm32f4xx_hal.h"
#include "stm32f4_discovery.h"
#include "stdbool.h"
#include "cmsis_os.h"

/******************************************************************************
Definitions
******************************************************************************/
/* PS2 Pin config */
#define PS2_SCK_PIN				GPIO_PIN_0
#define PS2_SCK_PORT			GPIOE
#define PS2_SCK_CLK_EN()		__GPIOE_CLK_ENABLE()  

#define PS2_SDA_PIN				GPIO_PIN_1
#define PS2_SDA_PORT            GPIOE
#define PS2_SDA_CLK_EN()        __GPIOE_CLK_ENABLE() 

#define PS2_DELAY_1MS()			HAL_Delay(1)//osDelay(1)

typedef enum{
	PS2_BUS_STATE_IDLE,
	PS2_BUS_STATE_INHIBIT,
	PS2_BUS_STATE_SEND,
	PS2_BUS_STATE_RECEIVE,
}PS2_StateTypedef;

typedef enum{
	PS2_SECTION_NONE,
	PS2_SECTION_CONTINUE,
}PS2_SectionStateTypedef;

typedef enum{
	PS2_ERROR,
	PS2_SUCCESS
}PS2_ResultTypedef;

typedef struct {
	bool IsSendDone;
	bool IsRecvAck;
	bool IsSectionDone;
}PS2_StatusTypedef;
	
#define PS2_INHIBIT_TIMEOUT		250
#define PS2_DEVICE_TIMEOUT		200
#define PS2_NEXTBIT_TIMEOUT		100
#define PS2_SECTION_NEXT_FR_TIMEOUT		300

#define PS2_TIMER_X				TIM2
#define PS2_TIMER_ISR			TIM2_IRQHandler

/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/
/* Callback function regist functions */
void PS2_RegistSectionDoneCb(void (*f)(const uint8_t* bytes, uint8_t n));
void PS2_RegistReceivedAckCb(void (*f)(void));


void PS2_Init(void);
void PS2_SendByte(uint8_t byte);
bool PS2_SendByteAndWaitAck(uint8_t byte);
bool PS2_SendByteAndWaitSectionEnd(uint8_t byte);


/* Bus ISR */
void PS2_SCKFallingEdgeHandler(void);
void PS2_TimerExpire(void);

#endif /* PS2_IO__ */

/******************* (C) COPYRIGHT DUYNV  **********************END OF FILE****/
