/**
  ******************************************************************************
  * @file    : UART.h
  * @author  : DuyNV
  * @version : V1.0
  * @date    : 22/02/2016
  * @brief   :
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SERIAL_H
#define __SERIAL_H

/******************************************************************************
Includes
******************************************************************************/
#include "fifo.h"
#include "STM32f4xx_hal.h"

/******************************************************************************
Definitions
******************************************************************************/

struct serial_uart_regs
{
	USART_TypeDef *uart;
	IRQn_Type irq;
	__IO uint32_t *apbx;
	uint32_t periph;
};

struct serial_io_conf
{
	uint8_t alternate;
	GPIO_TypeDef *port;
	uint32_t tx_pin;
	uint32_t rx_pin;
};


struct serial_com_prof{
	struct serial_uart_regs regs;
	struct serial_io_conf io_conf;
	fifo_t *tx_fifo;
	fifo_t *rx_fifo;
};

//struct serial_handle* serial_com_list[] = {&H_COM2, &H_COM3};
typedef enum {COM1, COM2, COM3, COM4} serial_comid;

typedef enum {SERIAL_NG = 0, SERIAL_OK = 1} SERIAL_RESULT;

/*
  COM CONFIGUARATION
  +-------+-----------+-------------+--------------+--------------+
  | COMx  |    AFx    |    GPIOx    |    TX_Pin    |    RX_Pin    |
  +-------+-----------+-------------+--------------+--------------+
  | COM1  |    AF7    |      B      |      6       |       7      |
  | COM2  |    AF7    |      A      |      2       |       3      |
  | COM3  |    AF7    |      B      |     10       |      11      |
  | COM4  |    AF8    |      A      |      0       |       1      |
  +-------+-----------+-------------+--------------+--------------+
*/

/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/
SERIAL_RESULT serial_open(serial_comid comid, uint32_t baund_rate);
SERIAL_RESULT serial_close(serial_comid comid);
uint16_t serial_read(serial_comid comid, char *buff, uint16_t len);
uint16_t serial_write(serial_comid comid, const char *buff, uint16_t len);




#endif /* __SERIAL_H */

/******************* (C) COPYRIGHT DUYNV  **********************END OF FILE****/
