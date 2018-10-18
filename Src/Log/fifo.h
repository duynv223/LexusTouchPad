/**
  ******************************************************************************
  * @file    :	fifo.h
  * @author  :	DuyNV
  * @version :	V1.0
  * @date    :	22/02/2016
  * @brief   :	
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FIFO_H
#define __FIFO_H
#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/

/* Constant definitions ------------------------------------------------------*/

/* Struct & Enum definitions -------------------------------------------------*/
typedef struct {
     char * buf;
     int head;
     int tail;
     int size;
} fifo_t;

/* Exported variable ---------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void fifo_init(fifo_t * f, char * buf, int size);
int fifo_read(fifo_t * f, void * buf, int nbytes);
int fifo_write(fifo_t * f, const void * buf, int nbytes);


#endif /* __FIFO_H */

/******************* (C) COPYRIGHT DUYNV  **********************END OF FILE****/
