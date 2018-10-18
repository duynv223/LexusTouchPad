/**
  ******************************************************************************
  * @file    Templates/Src/main.c 
  * @author  MCD Application Team
  * @version V1.2.4
  * @date    06-May-2016
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
osThreadId LEDThread1Handle, LEDThread2Handle, CANReporterThreadHandle, KeyPollingThreadHandle;

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);

static void LED_Thread1(void const *argument);
static void LED_Thread2(void const *argument);
static void KeyPolling_Thread(void const *argument);
static void CANReporter_Thread(void const *argument);


osMessageQId UserActionEvent;
osMessageQDef(osqueue, 160, void*);

osPoolId  mpool;
osPoolDef(mpool, 160, UserAction_Typedef); 





/* Private functions ---------------------------------------------------------*/

void MotionPackageReportHandler(Touchpad_MotionPackageTypedef pkg)
{
	//static int cnt = 0;
	osStatus status;
	//if(cnt++ %50 != 0)
		//return;
	UserAction_Typedef *pAction;
	
	log("%d %d %d %d %d %d\r\n", pkg.Finger, pkg.Gesture, pkg.Righ, pkg.Left, pkg.XPos, pkg.YPos);	
	pAction = osPoolAlloc(mpool);
	if(pAction)
	{
		pAction->type = USER_ACTION_TYPE_TOUCHPAD;
		//memcpy(pAction->data, &pkg, sizeof(Touchpad_MotionPackageTypedef));
		BSP_LED_On(LED6);
		//status = osMessagePut(UserActionEvent, (uint32_t)pAction, 0);
		if(status != osOK)
			BSP_LED_On(LED6);
	}
	else
	{
		BSP_LED_On(LED6);
	}
		
}

void CANLogMsg(CanRxMsgTypeDef *msg)
{
	//if(msg->StdId == 496)
//		//return;
//	//log("%d:%0.2x-%0.2x-%0.2x-%0.2x-%0.2x-%0.2x-%0.2x-%0.2x\r\n",
//		msg->StdId,
//		msg->Data[0],
//		msg->Data[1],
//		msg->Data[2],
//		msg->Data[3],
//		msg->Data[4],
//		msg->Data[5],
//		msg->Data[6],
//		msg->Data[7]);
//	//log("can handler\r\n");
}


/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{

  /* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, Flash preread and Buffer caches
       - Systick timer is configured by default as source of time base, but user 
             can eventually implement his proper time base source (a general purpose 
             timer for example or other time source), keeping in mind that Time base 
             duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
             handled in milliseconds basis.
       - Low Level Initialization
     */
  HAL_Init();

  /* Configure the system clock to 168 MHz */
  SystemClock_Config();
	
	/* Enable GPIO clock */
	__GPIOA_CLK_ENABLE();
	__GPIOB_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();
	__GPIOE_CLK_ENABLE();
	
	/* LED Initial */
	BSP_LED_Init(LED3);
	BSP_LED_Init(LED4);
	BSP_LED_Init(LED5);
	BSP_LED_Init(LED6);
	
	/* Threat 1 definition */
	osThreadDef(LED1, LED_Thread1, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	
	/* Threat 2 definition */
	osThreadDef(LED2, LED_Thread2, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);

	/* Threat 3 definition */
	osThreadDef(_CANReporter_Thread, CANReporter_Thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);

	
	/* Threat 4 definition */
	osThreadDef(_KeyPolling_Thread, KeyPolling_Thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	
	/* Create thread 1 */
	LEDThread1Handle = osThreadCreate(osThread(LED1), NULL);
	
	/* Create thread 2 */
	LEDThread2Handle = osThreadCreate(osThread(LED2), NULL);

	/* Create thread 3 */
	CANReporterThreadHandle = osThreadCreate(osThread(_CANReporter_Thread), NULL);

	KeyPollingThreadHandle = osThreadCreate(osThread(_KeyPolling_Thread), NULL);

	/* Creat user action event */
	UserActionEvent = osMessageCreate(osMessageQ(osqueue), NULL);

	/* Creat pool */ 
	mpool = osPoolCreate(osPool(mpool));



	/* open serial COM1 for log*/
	serial_open(COM1, 9600);
	
	commandConsoleStart(COM2, configMINIMAL_STACK_SIZE * 8, osPriorityBelowNormal);
	vRegisterSampleCLICommands();
	
	Touchpad_RegistMotionPkgReportHandler(MotionPackageReportHandler);
	CAN_RegistReceivedMsgHandler(CANLogMsg);
	
	CAN_Init();
	Key_Init();
	
	BSP_LED_Off(LED6);
  /* Start Scheduler */
	osKernelStart();
	
	
  /* Infinite loop */
  while (1)
  {
  }
}


static void LED_Thread1(void const *argument)
{
	//uint32_t count = 0;
	for(;;)
	{
		BSP_LED_Toggle(LED3);
		osDelay(100);
	}
}


static void LED_Thread2(void const *argument)
{

	uint32_t count = 0;
	osDelay(1000);
	Touchpad_Init();
	for(;;)
	{
		//log("Led 2 toggle time %d \r\n", count);
		count++;
		osDelay(1000);
	}
}

static void CANReporter_Thread(void const *argument)
{
	static CanTxMsgTypeDef msgKey;
	static CanTxMsgTypeDef msgTouchPad;
	osEvent event;
	UserAction_Typedef *pAction;
	for(;;)
		{
			BSP_LED_Toggle(LED4);
			/* Wait for new event: key down or touch pad */
			event = osMessageGet(UserActionEvent, 1000);
			log("e:%d\r\n", event.status);
			if(event.status == osEventMessage)
				{
					
					pAction = event.value.p;
					log("rcv:%x\r\n", (void *)pAction);
					if(pAction->type == USER_ACTION_TYPE_KEY)
						{
							log("USER_ACTION_TYPE_KEY\r\n %d", pAction->data[0]);
							
							msgKey.StdId = 0x1F1;
							msgKey.ExtId = 1;
							msgKey.RTR = CAN_RTR_DATA;
							msgKey.IDE = CAN_ID_STD;
							msgKey.DLC = 1;
							msgKey.Data[0] = pAction->data[0];
							CAN_Transmit(&msgKey);
						}
					else if(pAction->type == USER_ACTION_TYPE_TOUCHPAD)
						{
							log("USER_ACTION_TYPE_TOUCHPAD \r\n");
						}
					else
						{
							log("OTHER \r\n");
						}

					 osPoolFree(mpool, pAction);
				}
			else
				{
//					log("osEventTimeout \r\n");
//					msgKey.StdId = 0x1F1;
//					msgKey.ExtId = 1;
//					msgKey.RTR = CAN_RTR_DATA;
//					msgKey.IDE = CAN_ID_STD;
//					msgKey.DLC = 1;
//					msgKey.Data[0] = pAction->data[0];
//					CAN_Transmit(&msgKey);
				}
			/* Send to CAN bus */
		}
	
}


static void KeyPolling_Thread(void const *argument)
{
	static uint8_t preKey;
	uint8_t key;
	UserAction_Typedef *pAction;
	for(;;)
		{
			key = Key_Read();
			if(key != preKey)
			{
				pAction = osPoolAlloc(mpool);
				pAction->type = USER_ACTION_TYPE_KEY;
				pAction->data[0] = key;
				osMessagePut(UserActionEvent, (uint32_t)pAction, 0);
				preKey = key;
			}
			osDelay(10);
		}

	osThreadYield(); 
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported  */
  if (HAL_GetREVID() == 0x1001)
  {
    /* Enable the Flash prefetch */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  PS2_TimerExpire();
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	PS2_SCKFallingEdgeHandler();
}

void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* CanHandle)
{
	CAN_RxCpltHandler();
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
