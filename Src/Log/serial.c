/**
  ******************************************************************************
  * @file    : serial.c
  * @author  : DuyNV4
  * @version : V1.0
  * @date    : 22/02/2016
  * @brief   : This file provide firmware functions to manage serial port by name
  *            of COMx;
  			   + serial_open: open a com port
  			   + serial_close: close a comport
  			   + serial_write: write to serial
  			   + serial_read: read from serial
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/******************************************************************************
Includes
******************************************************************************/
#include "stm32f4xx_hal.h"
#include "serial.h"
#include "fifo.h"
#include "cmsis_os.h"

/******************************************************************************
Private Definitions
******************************************************************************/
#define __UART_ENABLE_IT(__UARTx__, __INTERRUPT__)   ((((__INTERRUPT__) >> 28U) == 1U)? ((__UARTx__)->CR1 |= ((__INTERRUPT__) & UART_IT_MASK)): \
                                                           (((__INTERRUPT__) >> 28U) == 2U)? ((__UARTx__)->CR2 |=  ((__INTERRUPT__) & UART_IT_MASK)): \
                                                        ((__UARTx__)->CR3 |= ((__INTERRUPT__) & UART_IT_MASK)))


#define __UART_DISABLE_IT(__UARTx__, __INTERRUPT__)  ((((__INTERRUPT__) >> 28U) == 1U)? ((__UARTx__)->CR1 &= ~((__INTERRUPT__) & UART_IT_MASK)): \
                                                           (((__INTERRUPT__) >> 28U) == 2U)? ((__UARTx__)->CR2 &= ~((__INTERRUPT__) & UART_IT_MASK)): \
                                                        ((__UARTx__)->CR3 &= ~ ((__INTERRUPT__) & UART_IT_MASK)))


#define __RCC_CLK_ENABLE(__APBx__, __PERPH__)		(__APBx__) |= (__PERPH__)
#define __RCC_CLK_DISABLE(__APBx__, __PERPH__)		(__APBx__) &= ~(__PERPH__)


#define serial_malloc 		pvPortMalloc
#define serial_free			vPortFree


/* UARTx relate register generator */
#define UARTx_REGS(__USARTx__, __APBx__)		{__USARTx__, __USARTx__##_IRQn, ((uint32_t *)RCC + offsetof(RCC_TypeDef, __APBx__##ENR)/4), RCC_##__APBx__##ENR_##__USARTx__##EN}

/* UARTx IO register generator */
#define UARTx_IO(__USARTx__, __AF__, __GPIOx__, __TXPIN__, __RXPIN__)		{GPIO_##__AF__##_##__USARTx__, __GPIOx__, GPIO_##__TXPIN__, GPIO_##__RXPIN__}



/* COM PORT PROFILE LIST */
struct serial_com_prof H_COM1 = {
	UARTx_REGS(USART1, APB2), 
	UARTx_IO(USART1, AF7, GPIOB, PIN_6, PIN_7),
	0, 0
};


struct serial_com_prof H_COM2 = {
	UARTx_REGS(USART2, APB1), 
	UARTx_IO(USART2, AF7, GPIOA, PIN_2, PIN_3),
	0, 0
};

struct serial_com_prof H_COM3 = {
	UARTx_REGS(USART3, APB1), 
	UARTx_IO(USART3, AF7, GPIOB, PIN_10, PIN_11),
	0, 0
};

struct serial_com_prof H_COM4 = {
	UARTx_REGS(UART4, APB1), 
	UARTx_IO(UART4, AF8, GPIOA, PIN_0, PIN_1),
	0, 0
};


struct serial_com_prof* serial_com_list[] = {&H_COM1, &H_COM2, &H_COM3, &H_COM4};




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


/*******************************************************************************
* Function Name  : 
* Description    : 
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
SERIAL_RESULT serial_open(serial_comid comid, uint32_t baund_rate)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	UART_HandleTypeDef uartHandle;
	fifo_t *tx_fifo;
	fifo_t *rx_fifo;
	uint8_t *tx_buff;
	uint8_t *rx_buff;

	/* Step 1: Enable peripherals and GPIO Clocks -------------------------------*/
	__RCC_CLK_ENABLE(*((volatile uint32_t *)(serial_com_list[comid]->regs.apbx)), serial_com_list[comid]->regs.periph);
	
	//__HAL_RCC_USART2_CLK_ENABLE();
	/* Step 2: Configure peripheral GPIO ----------------------------------------*/  
	/* --- TX */
	GPIO_InitStruct.Pin       = serial_com_list[comid]->io_conf.tx_pin;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_NOPULL;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
	GPIO_InitStruct.Alternate = serial_com_list[comid]->io_conf.alternate;
	HAL_GPIO_Init(serial_com_list[comid]->io_conf.port, &GPIO_InitStruct);

	/* --- RX */
	GPIO_InitStruct.Pin       = serial_com_list[comid]->io_conf.rx_pin;
	HAL_GPIO_Init(serial_com_list[comid]->io_conf.port, &GPIO_InitStruct);

	
	/* Step 3: Configure UART ---------------------------------------------------*/  
	/* Default: 8-Bits, 1-Stop-Bit, Parity-None, HWC-Disable */
	uartHandle.Instance 		 = serial_com_list[comid]->regs.uart;
	uartHandle.Init.BaudRate	 = baund_rate;
	uartHandle.Init.WordLength	 = UART_WORDLENGTH_8B;
	uartHandle.Init.StopBits	 = UART_STOPBITS_1;
	uartHandle.Init.Parity		 = UART_PARITY_NONE;
	uartHandle.Init.HwFlowCtl	 = UART_HWCONTROL_NONE;
	uartHandle.Init.Mode		 = UART_MODE_TX_RX;
	uartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
	  
	if(HAL_UART_Init(&uartHandle) != HAL_OK)
	{
	  //LOG_ERR_HANDLER();
	}

	/* Step 3: Configure Interrupt -----------------------------------------------*/ 
	HAL_NVIC_SetPriority(serial_com_list[comid]->regs.irq, 6, 0);
	HAL_NVIC_EnableIRQ(serial_com_list[comid]->regs.irq);

	
	/* Step 4: Allocate memory for fifo ------------------------------------------*/ 
	tx_fifo = (fifo_t *)serial_malloc(sizeof(fifo_t));
	rx_fifo = (fifo_t *)serial_malloc(sizeof(fifo_t));

	tx_buff = (uint8_t *)serial_malloc(500);
	rx_buff = (uint8_t *)serial_malloc(500);
	
	fifo_init(tx_fifo, (char *)tx_buff, 500);
	fifo_init(rx_fifo, (char *)rx_buff, 500);

	serial_com_list[comid]->tx_fifo = tx_fifo;
	serial_com_list[comid]->rx_fifo = rx_fifo;

	/* Enable rx interrupt */
	__UART_ENABLE_IT(serial_com_list[comid]->regs.uart, UART_IT_RXNE);
	return SERIAL_OK;
}

                                              

SERIAL_RESULT serial_close(serial_comid comid)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	/* Step 1: Disable peripherals and GPIO Clocks? -------------------------------*/
	__RCC_CLK_DISABLE(*((volatile uint32_t *)(serial_com_list[comid]->regs.apbx)), serial_com_list[comid]->regs.periph);
	/* Step 2: Deinit peripheral GPIO ---------------------------------------------*/
		/* --- TX */
	GPIO_InitStruct.Pin       = serial_com_list[comid]->io_conf.tx_pin;
	GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull      = GPIO_NOPULL;
	GPIO_InitStruct.Speed     = GPIO_SPEED_LOW;
	GPIO_InitStruct.Alternate = serial_com_list[comid]->io_conf.alternate;
	HAL_GPIO_Init(serial_com_list[comid]->io_conf.port, &GPIO_InitStruct);

	/* --- RX */
	GPIO_InitStruct.Pin       = serial_com_list[comid]->io_conf.rx_pin;
	HAL_GPIO_Init(serial_com_list[comid]->io_conf.port, &GPIO_InitStruct);
	/* Step 3: Deinit UART --------------------------------------------------------*/
		/* No need to do */
	/* Step 4: De-allocate memory for fifo ----------------------------------------*/
	serial_free(serial_com_list[comid]->tx_fifo->buf);
	serial_free(serial_com_list[comid]->rx_fifo->buf);
	serial_free(serial_com_list[comid]->tx_fifo);
	serial_free(serial_com_list[comid]->rx_fifo);
	serial_com_list[comid]->tx_fifo = (fifo_t *)0;
	serial_com_list[comid]->rx_fifo = (fifo_t *)0;
	
	/* Disable tx, rx interrupt */
	__UART_DISABLE_IT(serial_com_list[comid]->regs.uart, UART_IT_RXNE);
	__UART_DISABLE_IT(serial_com_list[comid]->regs.uart, UART_IT_TXE);
	return SERIAL_OK;
	
}

uint16_t serial_read(serial_comid comid, char *buff, uint16_t len)
{
	return fifo_read(serial_com_list[comid]->rx_fifo, (void *)buff, len);
}


uint16_t serial_write(serial_comid comid, const char *buff, uint16_t len)
{
	uint16_t count = fifo_write(serial_com_list[comid]->tx_fifo, (void *)buff, len);
	__UART_ENABLE_IT(serial_com_list[comid]->regs.uart, UART_IT_TXE);
	return count;
}

void serialx_isr(serial_comid comid)
{
	char chr;
	if((serial_com_list[comid]->tx_fifo != 0) && ((serial_com_list[comid]->regs.uart->SR & USART_SR_TXE) != 0u))
	{
		if(fifo_read(serial_com_list[comid]->tx_fifo, &chr, 1) == 1)
		{
			serial_com_list[comid]->regs.uart->DR = chr & (uint16_t)0x01FFU;
		}
		else
		{
			__UART_DISABLE_IT(serial_com_list[comid]->regs.uart, UART_IT_TXE);
		}
	}

	if((serial_com_list[comid]->rx_fifo != 0) && ((serial_com_list[comid]->regs.uart->SR & USART_SR_RXNE) != 0u))
	{
		chr = (uint16_t)(serial_com_list[comid]->regs.uart->DR & (uint16_t)0x01FFU);
		fifo_write(serial_com_list[comid]->rx_fifo, &chr, 1);
	}

	
}


void USART1_IRQHandler(void)
{
	serialx_isr(COM1);
}


void USART2_IRQHandler(void)
{
	serialx_isr(COM2);
}


void USART3_IRQHandler(void)
{
	serialx_isr(COM3);
}

void UART4_IRQHandler(void)
{
	serialx_isr(COM4);
}


/******************* (C) COPYRIGHT DUYNV  **********************END OF FILE****/
