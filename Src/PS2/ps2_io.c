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
#include "ps2_io.h"
/******************************************************************************
Private Definitions
******************************************************************************/


/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/

/******************************************************************************
Private global variables
******************************************************************************/
GPIO_InitTypeDef  	g_GPIO_InitStruct;
TIM_HandleTypeDef 	g_TimHandle;

PS2_StateTypedef g_PS2State;
PS2_StatusTypedef g_PS2Status;

bool g_ReceivedAck;

uint8_t g_BitCnt;
uint8_t g_ByteToSend;
uint8_t g_ByteToReceive;
uint8_t g_ReceivedBytes[10];
uint8_t g_ReceivedByteCnt;
uint8_t g_OddParity;

void (*gp_ReceivedAckCb)(void);
void (*gp_SectionDoneCb)(const uint8_t* bytes, uint8_t n);


/******************************************************************************
Private function prototypes
******************************************************************************/
void PS2_StartTimer(uint32_t us);
/******************************************************************************
Private functions
******************************************************************************/


/* PS2 IO Operation -------------------------------------------------------------------------------*/
GPIO_PinState PS2_SDA_Get()
{
	return HAL_GPIO_ReadPin(PS2_SDA_PORT, PS2_SDA_PIN);
}

void PS2_SDA_SetL()
{
	HAL_GPIO_WritePin(PS2_SDA_PORT, PS2_SDA_PIN, GPIO_PIN_RESET);
}

void PS2_SDA_SetH()
{
	HAL_GPIO_WritePin(PS2_SDA_PORT, PS2_SDA_PIN, GPIO_PIN_SET);
}

void PS2_SCK_SetL()
{
	HAL_GPIO_WritePin(PS2_SCK_PORT, PS2_SCK_PIN, GPIO_PIN_RESET);
}

void PS2_SCK_SetH()
{
	HAL_GPIO_WritePin(PS2_SCK_PORT, PS2_SCK_PIN, GPIO_PIN_SET);
}

void PS2_SDA_ModeI()
{
	g_GPIO_InitStruct.Pin = PS2_SDA_PIN; 
	g_GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	g_GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	g_GPIO_InitStruct.Pull  = GPIO_PULLUP;
	HAL_GPIO_Init(PS2_SDA_PORT, &g_GPIO_InitStruct); 
}

void PS2_SDA_ModeO()
{
	g_GPIO_InitStruct.Pin = PS2_SDA_PIN; 
	g_GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	g_GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	g_GPIO_InitStruct.Pull  = GPIO_PULLUP;
	HAL_GPIO_Init(PS2_SDA_PORT, &g_GPIO_InitStruct); 
}

void PS2_SCK_ModeI()
{
	g_GPIO_InitStruct.Pin = PS2_SCK_PIN; 
	g_GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	g_GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	g_GPIO_InitStruct.Pull  = GPIO_PULLUP;
	HAL_GPIO_Init(PS2_SCK_PORT, &g_GPIO_InitStruct); 
	HAL_NVIC_DisableIRQ(EXTI0_IRQn);
}

void PS2_SCK_ModeO()
{
	g_GPIO_InitStruct.Pin = PS2_SCK_PIN; 
	g_GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	g_GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	g_GPIO_InitStruct.Pull  = GPIO_PULLUP;
	HAL_GPIO_Init(PS2_SCK_PORT, &g_GPIO_InitStruct); 
	HAL_NVIC_DisableIRQ(EXTI0_IRQn);
}

void PS2_SCK_ModeITF()
{
	g_GPIO_InitStruct.Pin = PS2_SCK_PIN; 
	g_GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	g_GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	g_GPIO_InitStruct.Pull  = GPIO_PULLUP;
	HAL_GPIO_Init(PS2_SCK_PORT, &g_GPIO_InitStruct); 
	
	HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

void PS2_SCK_ModeITR()
{
	g_GPIO_InitStruct.Pin = PS2_SCK_PIN; 
	g_GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	g_GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	g_GPIO_InitStruct.Pull  = GPIO_PULLUP;
	HAL_GPIO_Init(PS2_SCK_PORT, &g_GPIO_InitStruct); 
	
	HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}



/* PS2 fucntion implementation --------------------------------------------------------------------*/
void PS2_RegistSectionDoneCb(void (*f)(const uint8_t* bytes, uint8_t n))
{
	gp_SectionDoneCb = f;
}

void PS2_RegistReceivedAckCb(void (*f)(void))
{
	gp_ReceivedAckCb = f;
}

static void PS2_BusError()
{
	
}

static void PS2_BusSendDone()
{
	g_PS2Status.IsSendDone = true;
}

static void PS2_BusRecvByte(uint8_t byte)
{
	/* ACK */
	if(byte == 0xFA)
		{
			g_PS2Status.IsRecvAck = true;
			if(gp_ReceivedAckCb)
				gp_ReceivedAckCb();
		}
	/* Data */
	else
		{
			g_ReceivedBytes[g_ReceivedByteCnt] = byte;
			g_ReceivedByteCnt++;
		}
}

static void PS2_SectionDone()
{
	g_PS2Status.IsSectionDone = true;
	if(gp_SectionDoneCb && g_ReceivedByteCnt != 0)
		gp_SectionDoneCb(g_ReceivedBytes, g_ReceivedByteCnt);
	
	g_ReceivedByteCnt = 0;
}



void PS2_Init(void)
{
	/* Config IO */
	PS2_SCK_CLK_EN();
	PS2_SDA_CLK_EN();
	
	PS2_SCK_ModeITF();
	PS2_SDA_ModeI();

	/* Config timer */
	__TIM2_CLK_ENABLE();
	g_TimHandle.Instance = PS2_TIMER_X;
	
	g_TimHandle.Init.Period = 10000 - 1;
	g_TimHandle.Init.Prescaler = (uint32_t) ((SystemCoreClock / 1000000) - 1);
	g_TimHandle.Init.ClockDivision = 0;
	g_TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
	
	HAL_TIM_Base_Init(&g_TimHandle);
	HAL_TIM_Base_Stop_IT(&g_TimHandle);

	HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);

	/* Enable the TIMx global Interrupt */
	HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

void PS2_SendByte(uint8_t byte)
{
	g_ByteToSend = byte;
	/* 0. drive bus */
	PS2_SDA_SetH();
	PS2_SCK_SetH();
	
	PS2_SDA_ModeO();
	PS2_SCK_ModeO();

	/* 1. pull down the clock line */
	PS2_SCK_SetL();

	/* 2. start timer for inhibit time */
	PS2_StartTimer(PS2_INHIBIT_TIMEOUT);

	/* 3. change to next state */
	g_PS2State = PS2_BUS_STATE_INHIBIT;
	g_PS2Status.IsRecvAck = false;
	g_PS2Status.IsSectionDone = false;
	g_PS2Status.IsSendDone = false;
}

bool WaitForFlag(bool *flag, uint8_t timeout)
{
	while(timeout-- && !*flag)
		{
			PS2_DELAY_1MS();
		}
	return *flag == true;
}

bool PS2_SendByteAndWaitAck(uint8_t byte)
{
	bool ret = false;
	PS2_SendByte(byte);
	if(WaitForFlag(&g_PS2Status.IsSendDone, 2))
		{
			if(WaitForFlag(&g_PS2Status.IsRecvAck, 3))
				{
					ret = true;
				}
		}
	return ret;
}

bool  PS2_SendByteAndWaitSectionEnd(uint8_t byte)
{
	bool ret = false;
	if(PS2_SendByteAndWaitAck(byte))
		{
			if(WaitForFlag(&g_PS2Status.IsRecvAck, 10))
				{
					ret = true;
				}
		}
	return ret;
}


void PS2_StartTimer(uint32_t us)
{
	g_TimHandle.Init.Period = us - 1;
	
	HAL_TIM_Base_Init(&g_TimHandle);
	g_TimHandle.Instance->CNT = 0;
	__HAL_TIM_CLEAR_IT(&g_TimHandle, TIM_IT_UPDATE);
	HAL_TIM_Base_Start_IT(&g_TimHandle);
}

void PS2_StopTimer(void)
{
	/* Disable timer interrupt */
	HAL_TIM_Base_Stop_IT(&g_TimHandle);

}


void PS2_TimerExpire(void)
{
	/* Disable timer interrupt */
	HAL_TIM_Base_Stop_IT(&g_TimHandle);

	/* handle timout */
	switch(g_PS2State)
		{
		case PS2_BUS_STATE_IDLE:
			/* Timout in idle state 
				--> timeout of continuous frame check 
				--> end of section
			*/
			PS2_SectionDone();
			break;
			
		case PS2_BUS_STATE_INHIBIT:
			/* pull data line to low and release clk line and enable clk line interrupt */
			PS2_SDA_SetL();
			PS2_SCK_ModeITF();

			/* change to next state */
			g_PS2State = PS2_BUS_STATE_SEND;

			/* set timer to check frame time out */
			PS2_StartTimer(PS2_DEVICE_TIMEOUT);
			
			/* prepaire for send */
			g_BitCnt = 0;
			g_OddParity = 1;
			break;
			
		case PS2_BUS_STATE_SEND:
		case PS2_BUS_STATE_RECEIVE:
			PS2_BusError();
			PS2_SCK_ModeITF();
			PS2_SDA_ModeI();
			g_PS2State = PS2_BUS_STATE_IDLE;
			break;
			
		default:
			break;
		}

	
}

void PS2_SCKFallingEdgeHandler(void)
{
	switch(g_PS2State)
	{
	case PS2_BUS_STATE_SEND:
		PS2_StartTimer(PS2_NEXTBIT_TIMEOUT);
		/* Latch the bit 0-7*/
		if(g_BitCnt < 8)
			{
				if(g_ByteToSend & (1 << g_BitCnt))
					{
						PS2_SDA_SetH();
						g_OddParity ^= 0x01;
					}
				else
					{
						PS2_SDA_SetL();
					}
			}
		/* Latch P bit */
		else if(g_BitCnt == 8)
			{
				if(g_OddParity)
					PS2_SDA_SetH();
				else
					PS2_SDA_SetL();
			}
		/* Latch S bit */
		else if(g_BitCnt == 9)
			{
				PS2_SDA_SetH();
			}
		/* End of frame */
		else if(g_BitCnt == 10)
			{
				PS2_BusSendDone();
				//BSP_LED_Toggle(LED3);
				g_PS2State = PS2_BUS_STATE_IDLE;
				PS2_SDA_ModeI();
				PS2_SCK_ModeITF();
			}

		PS2_StopTimer();
		g_BitCnt++;
		break;
		
	case PS2_BUS_STATE_IDLE:
		/* prepaire for new frame */
		g_ByteToReceive = 0x00;
		g_BitCnt = 0;

		/* goto next state */
		PS2_StartTimer(PS2_NEXTBIT_TIMEOUT);
		g_PS2State = PS2_BUS_STATE_RECEIVE;
		break;
		
	case PS2_BUS_STATE_RECEIVE:
		PS2_StopTimer();
		if(g_BitCnt < 8)
			{
				g_ByteToReceive >>= 1;
				g_ByteToReceive |= (PS2_SDA_Get() == GPIO_PIN_RESET? 0:0x80);
			}
		/* Recv P bit */
		else if(g_BitCnt == 8)
			{
				/* todo: check odd parity */
			}
		/* Recv S bit --> end*/
		else if(g_BitCnt == 9)
			{
				//BSP_LED_Toggle(LED3);
				PS2_BusRecvByte(g_ByteToReceive);

				/* Start timer to find end of section */
				PS2_StartTimer(PS2_SECTION_NEXT_FR_TIMEOUT);
				g_PS2State = PS2_BUS_STATE_IDLE;
			}
		
		PS2_StartTimer(PS2_NEXTBIT_TIMEOUT);
		g_BitCnt++;
		break;
		
	default:
		break;
	}
}

/******************* (C) COPYRIGHT DUYNV  **********************END OF FILE****/
