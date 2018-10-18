/**
  ******************************************************************************
  * @file    :	fifo.c
  * @author  :	DuyNV	
  * @version :	V1.0
  * @date    :	22/02/2016
  * @brief   :
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "fifo.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


/*******************************************************************************
* Function Name  : fifo_init
* Description    : Initial fifo
* Input          : [fifo_t *]f
									 [char *]buf
									 [int]size
* Output         : None
* Return         : None
* Attention		 	 : None
*******************************************************************************/
void fifo_init(fifo_t * f, char * buf, int size){
     f->head = 0;
     f->tail = 0;
     f->size = size;
     f->buf = buf;
}

/*******************************************************************************
* Function Name  : fifo_read
* Description    : fifo read out
* Input          : [fifo_t *]f
									 [int]nbytes
* Output         : [char *]buf
* Return         : Number of bytes read
* Attention		 	 : None
*******************************************************************************/
int fifo_read(fifo_t * f, void * buf, int nbytes){
     int i;
     char * p;
     p = buf;
     for(i=0; i < nbytes; i++){
          if( f->tail != f->head ){ //see if any data is available
               *p++ = f->buf[f->tail];  //grab a byte from the buffer
               f->tail++;  //increment the tail
               if( f->tail == f->size ){  //check for wrap-around
                    f->tail = 0;
               }
          } else {
               return i; //number of bytes read 
          }
     }
     return nbytes;
}

/*******************************************************************************
* Function Name  : fifo_write
* Description    : write up to fifo
* Input          : [fifo_t *]f
									 [char *]buf
									 [int]nbytes
* Output         : None
* Return         : Number of bytes written
* Attention		 	 : None
*******************************************************************************/
int fifo_write(fifo_t * f, const void * buf, int nbytes){
     int i;
     const char * p;
     p = buf;
     for(i=0; i < nbytes; i++){
           //first check to see if there is space in the buffer
           if( (f->head + 1 == f->tail) ||
                ( (f->head + 1 == f->size) && (f->tail == 0))){
                 return i; //no more room
           } else {
               f->buf[f->head] = *p++;
               f->head++;  //increment the head
               if( f->head == f->size ){  //check for wrap-around
                    f->head = 0;
               }
           }
     }
     return nbytes;
}


/******************* (C) COPYRIGHT DUYNV  **********************END OF FILE****/
