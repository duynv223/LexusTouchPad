/****************************************Copyright (c)**************************************************                         
**
**                                 http://www.powermcu.com
**
**--------------File Info-------------------------------------------------------------------------------
** File name:			GLCD.c
** Descriptions:		STM32 FSMC TFT操作函数库
**						
**------------------------------------------------------------------------------------------------------
** Created by:			poweravr
** Created date:		2010-11-7
** Version:				1.0
** Descriptions:		The original version
**
**------------------------------------------------------------------------------------------------------
** Modified by:			
** Modified date:	
** Version:
** Descriptions:		
********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
//#include <includes.h>

#include "AsciiLib.h"

#include "GLCD.h"
#include "LCDConf.h"
/* Private variables ---------------------------------------------------------*/
uint16_t DeviceCode;
//static uint16_t TimerPeriod = 0;
//static uint16_t Channel2Pulse = 1000;
static int MaskT, MaskL, MaskR, MaskB;	/* Active drawing area */
static int LocX, LocY;			/* Current dot position */
static uint32_t ChrColor;		/* Current character color ((bg << 16) + fg) */
static const uint8_t *FontS;	/* Current font (ANK) */

void LCD_Delay(uint16_t nCount);
static __inline void ili9341_WriteReg(uint8_t reg);
static __inline void ili9341_WriteData(uint16_t data);
static __inline void ili9341_WriteWord(uint16_t data);

/* Private define ------------------------------------------------------------*/
/* 使用总线方式时定义地址 */
/* 挂在不同的BANK,使用不同地址线时请自行换算地址 */

#define LCD_REG              (*((volatile unsigned short *) 0x60000000)) /* RS = 0 */
#define LCD_RAM              (*((volatile unsigned short *) 0x60020000)) /* RS = 1 */

//const uint8_t Font5x10[], Font8x16[];	/* Font image (FONTX2 format) */
//IMPORT_BIN("mplfont/MPLHN10X.FNT", Font5x10);	/* const uint8_t Font5x10[] */
//IMPORT_BIN_PART("fnt8x16.FNT", 0, 17 + 8 * 128, Font8x16);	/* const uint8_t Font8x16[] */

/*******************************************************************************
* Function Name  : LCD_CtrlLinesConfig
* Description    : Configures LCD Control lines (FSMC Pins) in alterna`te function
                   Push-Pull mode.
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/

void LCDPeripheralInit(void)
{
	/*-- GPIOs Configuration -----------------------------------------------------*/
	/*
	 +-------------------+--------------------+------------------+------------------+
	 +                       LCD pins assignment                                  +
	 +-------------------+--------------------+------------------+------------------+
	 | PD0  <-> FSMC_D2  |                    | PD11 <-> FSMC_A16| PD7 <-> FSMC_NE1 |
	 | PD1  <-> FSMC_D3  |                    |                  |                  |
	 | PD4  <-> FSMC_NOE | PE7  <-> FSMC_D4   |                  |                  |
	 | PD5  <-> FSMC_NWE | PE8  <-> FSMC_D5   |                  |                  |
	 | PD8  <-> FSMC_D13 | PE9  <-> FSMC_D6   |                  |                  |
	 | PD9  <-> FSMC_D14 | PE10 <-> FSMC_D7   |                  |                  |
	 | PD10 <-> FSMC_D15 | PE11 <-> FSMC_D8   |                  |                  |
	 |                   | PE12 <-> FSMC_D9   |                  |------------------+
	 |                   | PE13 <-> FSMC_D10  |                  |
	 | PD14 <-> FSMC_D0  | PE14 <-> FSMC_D11  |                  |
	 | PD15 <-> FSMC_D1  | PE15 <-> FSMC_D12  |------------------+
	 +-------------------+--------------------+
	   PC13  <-> RESET
	*/
	
	/* Refactor using hal library */
	SRAM_HandleTypeDef hsram;
	FSMC_NORSRAM_TimingTypeDef SRAM_Timing;
	
	hsram.Instance  = FSMC_NORSRAM_DEVICE;
  hsram.Extended  = FSMC_NORSRAM_EXTENDED_DEVICE;
  
  SRAM_Timing.AddressSetupTime       = 10;
  SRAM_Timing.AddressHoldTime        = 0;
  SRAM_Timing.DataSetupTime          = 10;
  SRAM_Timing.BusTurnAroundDuration  = 0;
  SRAM_Timing.CLKDivision            = 0;
  SRAM_Timing.DataLatency            = 0;
  SRAM_Timing.AccessMode             = FSMC_ACCESS_MODE_A;
  
  hsram.Init.NSBank             = FSMC_NORSRAM_BANK1;
  hsram.Init.DataAddressMux     = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram.Init.MemoryType         = FSMC_MEMORY_TYPE_NOR;
  hsram.Init.MemoryDataWidth    = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram.Init.BurstAccessMode    = FSMC_BURST_ACCESS_MODE_DISABLE;//
  hsram.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;//
  hsram.Init.WrapMode           = FSMC_WRAP_MODE_DISABLE;//
  hsram.Init.WaitSignalActive   = FSMC_WAIT_TIMING_BEFORE_WS;//
  hsram.Init.WriteOperation     = FSMC_WRITE_OPERATION_ENABLE;//
  hsram.Init.WaitSignal         = FSMC_WAIT_SIGNAL_DISABLE;//
  hsram.Init.ExtendedMode       = FSMC_EXTENDED_MODE_DISABLE;//
  hsram.Init.AsynchronousWait   = FSMC_ASYNCHRONOUS_WAIT_DISABLE;//
  hsram.Init.WriteBurst         = FSMC_WRITE_BURST_DISABLE;//
	
  hsram.State = HAL_SRAM_STATE_RESET;
	 /* Initialize the SRAM controller */
  if(HAL_SRAM_Init(&hsram, &SRAM_Timing, &SRAM_Timing) != HAL_OK)
  {
    /* Initialization Error */
		//printf("[LCD] FSMC configuaration error");
  }
	//else
		//printf("[LCD] FSMC configuaration success");
#if 0
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB3PeriphClockCmd(RCC_AHB3ENR_FSMCEN RCC_AHB3Periph_FSMC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOC, ENABLE);
	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);


	/* RESET */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
    /* RESET */
  //GPIO_InitStructure.GPIO_Pin=GPIO_Pin_13;
  //GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
  //GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
  //GPIO_Init(GPIOC,&GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_13 );

	/* GPIOD Configuration */
	/* PD.00(D2), PD.01(D3), PD.04(RD), PD.5(WR), PD.7(CS), PD.8(D13), PD.9(D14),
     PD.10(D15), PD.11(RS) PD.14(D0) PD.15(D1) */
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_4 |GPIO_Pin_5| 
								  GPIO_Pin_7| GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 | 
								  GPIO_Pin_11|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);
  
	/* GPIOE Configuration */


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7  | GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 | 
                                  GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | 
                                  GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

  GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource15, GPIO_AF_FSMC);
#endif
	/* RESET PIN Configuaration */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin = GPIO_PIN_13;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

}

/*******************************************************************************
* Function Name  : LCD_Configuration
* Description    : Configure the LCD Control pins and FSMC Parallel interface
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_Configuration(void)
{
	LCDPeripheralInit();
	LCD_Delay(20);
	//GPIO_ResetBits(GPIOC, GPIO_Pin_13 );
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
	LCD_Delay(6);
	//GPIO_SetBits(GPIOC, GPIO_Pin_13 );
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}





/*******************************************************************************
* Function Name  : LCD_WriteReg
* Description    : Writes to the selected LCD register.
* Input          : - LCD_Reg: address of the selected register.
*                  - LCD_RegValue: value to write to the selected register.
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __inline void LCD_WriteReg(uint8_t LCD_Reg,uint16_t LCD_RegValue)
{
  /* Write 16-bit Index, then Write Reg */
  LCD_REG = LCD_Reg;
  /* Write 16-bit Reg */
  LCD_RAM = LCD_RegValue;
}



/*******************************************************************************
* Function Name  : LCD_WriteReg
* Description    : Reads the selected LCD Register.
* Input          : None
* Output         : None
* Return         : LCD Register Value.
* Attention		 : None
*******************************************************************************/

static __inline uint16_t LCD_ReadReg(uint8_t LCD_Reg)
{
  /* Write 16-bit Index (then Read Reg) */
  LCD_REG = LCD_Reg;
  /* Read 16-bit Reg */
  return (LCD_RAM);
}

/*******************************************************************************
* Function Name  : LCD_WriteRAM_Prepare
* Description    : Prepare to write to the LCD RAM.
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __inline void LCD_WriteRAM_Prepare(void)
{
  ili9341_WriteReg(0x2C);
}

/*******************************************************************************
* Function Name  : LCD_WriteRAM
* Description    : Writes to the LCD RAM.
* Input          : - RGB_Code: the pixel color in RGB mode (5-6-5).
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __inline void LCD_WriteRAM(uint16_t RGB_Code)					 
{
  /* Write 16-bit GRAM Reg */
  LCD_RAM = RGB_Code;
}
void  LCD_Write_data(uint16_t RGB_Code)
{
	     LCD_RAM =RGB_Code;
}

/*******************************************************************************
* Function Name  : LCD_ReadRAM
* Description    : Reads the LCD RAM.
* Input          : None
* Output         : None
* Return         : LCD RAM Value.
* Attention		 : None
*******************************************************************************/
static __inline uint16_t LCD_ReadRAM(void)
{
  volatile uint16_t dummy; 
  /* Write 16-bit Index (then Read Reg) */
  LCD_REG = R34; /* Select GRAM Reg */
  /* Read 16-bit Reg */
  dummy = LCD_RAM; 
  
  return LCD_RAM;
}

//
/******************************************************************************
* Function Name  : LCD_SetWindows
* Description    : Sets Windows Area.
* Input          : - StartX: Row Start Coordinate 
*                  - StartY: Line Start Coordinate  
*				   - xLong:  
*				   - yLong: 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/

 void disp_setrect (
	int left,		
	int right,	
	int top,		
	int bottom	
)
{
	//
#if 0
	  LCD_WriteReg(0x0044,(bottom<<8)|top);        
    LCD_WriteReg(0x0045,319-right);         
    LCD_WriteReg(0x0046,319-left); 
  	LCD_WriteReg(0x004e,top); 
    LCD_WriteReg(0x004f,319-left); 	
    LCD_REG = R34;  
#elseif
	
	
	  LCD_WriteReg(0x0050, top);
    LCD_WriteReg(0x0051, bottom);         
    LCD_WriteReg(0x0052, 319 - right); 
  	LCD_WriteReg(0x0053, 319 - left); 
	
#endif
	  return;
}

/*-----------------------------------------------------*/
/* Set active drawing area                             */
/*-----------------------------------------------------*/
/* The mask feature affects only disp_fill, disp_box,  */
/* disp_pset, disp_lineto and disp_blt function        */

void disp_mask (
	int left,		/* Left end of active window (0 to DISP_XS-1) */
	int right,		/* Right end of active window (0 to DISP_XS-1, >=left) */
	int top,		/* Top end of active window (0 to DISP_YS-1) */
	int bottom		/* Bottom end of active window (0 to DISP_YS-1, >=top) */
)
{
	if (left >= 0 && right < DISP_XS && left <= right && top >= 0 && bottom < DISP_YS && top <= bottom) {
		MaskL = left;
		MaskR = right;
		MaskT = top;
		MaskB = bottom;
	}
}
//
//
void disp_fill (
	int left,		/* Left end (-32768 to 32767) */
	int right,		/* Right end (-32768 to 32767, >=left) */
	int top,		/* Top end (-32768 to 32767) */
	int bottom,		/* Bottom end (-32768 to 32767, >=top) */
	uint16_t color	/* Box color */
)
{
	volatile uint32_t n;

  
	if (left > right || top > bottom) return; 	
	if (left > MaskR || right < MaskL  || top > MaskB || bottom < MaskT) return;	

	if (top < MaskT) top = MaskT;		
	if (bottom > MaskB) bottom = MaskB;
	if (left < MaskL) left = MaskL;		
	if (right > MaskR) right = MaskR;	
  
	disp_setrect(left, right, top, bottom);
	n = (uint32_t)(right - left + 1) * (uint32_t)(bottom - top + 1);
	do { LCD_RAM=color; } while (--n);
}
//
//
/*-----------------------------------------------------*/
/* Draw a hollow rectangular                           */

void disp_box (
	int left,		/* Left end (-32768 to 32767) */
	int right,		/* Right end (-32768 to 32767, >=left) */
	int top,		/* Top end (-32768 to 32767) */
	int bottom,		/* Bottom end (-32768 to 32767, >=top) */
	uint16_t color	/* Box color */
)
{
	disp_fill(left, left, top, bottom, color);
	disp_fill(right, right, top, bottom, color);
	disp_fill(left, right, top, top, color);
	disp_fill(left, right, bottom, bottom, color);
}
/*-----------------------------------------------------*/
/* Draw a dot                                          */
	
void disp_pset (
	int x,		/* X position (-32768 to 32767) */
	int y,		/* Y position (-32768 to 32767) */
	uint16_t color	/* Pixel color */
)
{
	if (x >= MaskL && x <= MaskR && y >= MaskT && y <= MaskB) {
	//
	  LCD_WriteReg(0x0044,(y<<8)|y);        
    LCD_WriteReg(0x0045,319-x);         
    LCD_WriteReg(0x0046,319-x); 
  	LCD_WriteReg(0x004e,y); 
    LCD_WriteReg(0x004f,319-x); 	

    LCD_REG = R34;
		LCD_RAM=color;
	}
}

/*******************************************************************************
* Set Line	                                                              		 *
*   Parameter:    s:  number of sector								                         *
*									*dat: data																									 *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_SetLine (unsigned short l, unsigned short *dat) {
	int i;
	disp_setrect (0,319,l,l);		
	for(i = 0; i < 320; i++)
	{
		LCD_RAM=dat[i]; 
	}
}
/*-----------------------------------------------------*/
/* Set current dot position for disp_lineto            */

void disp_moveto (
	int x,		/* X position (-32768 to 32767) */
	int y		/* Y position (-32768 to 32767) */
)
{
	LocX = x;
	LocY = y;
}



/*-----------------------------------------------------*/
/* Draw a line from current position                   */

void disp_lineto (
	int x,		/* X position for the line to (-32768 to 32767) */
	int y,		/* Y position for the line to (-32768 to 32767) */
	uint16_t col	/* Line color */
)
{
	int32_t xr, yr, xd, yd;
	int ctr;


	xd = x - LocX; xr = LocX << 16; LocX = x;
	yd = y - LocY; yr = LocY << 16; LocY = y;

	if ((xd < 0 ? 0 - xd : xd) >= (yd < 0 ? 0 - yd : yd)) {
		ctr = (xd < 0 ? 0 - xd : xd) + 1;
		yd = (yd << 16) / (xd < 0 ? 0 - xd : xd);
		xd = (xd < 0 ? -1 : 1) << 16;
	} else {
		ctr = (yd < 0 ? 0 - yd : yd) + 1;
		xd = (xd << 16) / (yd < 0 ? 0 - yd : yd);
		yd = (yd < 0 ? -1 : 1) << 16;
	}
	xr += 1 << 15;
	yr += 1 << 15;
	do {
		disp_pset(xr >> 16, yr >> 16, col);
		xr += xd; yr += yd;
	} while (--ctr);

}
/*-----------------------------------------------------*/
/* Copy image data to the display                      */

void disp_blt ( 
	int left,		/* Left end (-32768 to 32767) */
	int right,		/* Right end (-32768 to 32767, >=left) */
	int top,		/* Top end (-32768 to 32767) */
	int bottom,		/* Bottom end (-32768 to 32767, >=right) */
	const uint16_t *pat	/* Pattern data */
)
{
	volatile int yc, xc, xl, xs;
	uint16_t pd;


	if (left > right || top > bottom) return; 	/* Check varidity */
	if (left > MaskR || right < MaskL  || top > MaskB || bottom < MaskT) return;	/* Check if in active area */

	yc = bottom - top + 1;			/* Vertical size */
	xc = right - left + 1; xs = 0;	/* Horizontal size and skip */

	if (top < MaskT) {		/* Clip top of source image if it is out of active area */
		pat += xc * (MaskT - top);
		yc -= MaskT - top;
		top = MaskT;
	}
	if (bottom > MaskB) {	/* Clip bottom of source image if it is out of active area */
		yc -= bottom - MaskB;
		bottom = MaskB;
	}
	if (left < MaskL) {		/* Clip left of source image if it is out of active area */
		pat += MaskL - left;
		xc -= MaskL - left;
		xs += MaskL - left;
		left = MaskL;
	}
	if (right > MaskR) {	/* Clip right of source image it is out of active area */
		xc -= right - MaskR;
		xs += right - MaskR;
		right = MaskR;
	}

	disp_setrect(left, right, top, bottom);	/* Set rectangular area to fill */
	do {	/* Send image data */
		xl = xc;
		do {
			pd = *pat++;
      LCD_RAM=pd;
		} while (--xl);
		pat += xs;
	} while (--yc);

}
//
/*-----------------------------------------------------*/
/* Set current character position for disp_putc        */

void disp_locate (
	int col,	/* Column position */
	int row		/* Row position */
)
{
	if (FontS) {	/* Pixel position is calcurated with size of single byte font */
		LocX = col * FontS[14];
		LocY = row * FontS[15];
#if DISP_USE_DBCS
		Sjis1 = 0;
#endif
	}
}
/*-----------------------------------------------------*/
/* Register text font                                  */

void disp_font_face (
	const uint8_t *font	/* Pointer to the font structure in FONTX2 format */
)
{
	if (!memcmp(font, "FONTX2", 5)) {
#if DISP_USE_DBCS
		if (font[16] != 0)
			FontD = font;
		else
#endif
			FontS = font;
	}
}
/*-----------------------------------------------------*/
/* Set current text color                              */

void disp_font_color (
	uint32_t color	/* (bg << 16) + fg */
)
{
	ChrColor = color;
}
/*-----------------------------------------------------*/
/* Put a text character                                */

void disp_putc (
	uint8_t chr		/* Character to be output (kanji chars are given in two disp_putc sequence) */
)
{
	const uint8_t *fnt;
	uint8_t b, d;
	uint16_t dchr;
	uint32_t col;
	int h, wc, w, wb, i, fofs;


	if ((fnt = FontS) == 0) return;	/* Exit if no font registerd */

	if (chr < 0x20) {	/* Processes the control character */
#if DISP_USE_DBCS
		Sjis1 = 0;
#endif
		switch (chr) {
		case '\n':	/* LF */
			LocY += fnt[15];
			/* follow next case */
		case '\r':	/* CR */
			LocX = 0;
			return;
		case '\b':	/* BS */
			LocX -= fnt[14];
			if (LocX < 0) LocX = 0;
			return;
		case '\f':	/* FF */
			disp_fill(0, DISP_XS - 1, 0, DISP_YS - 1, 0);
			LocX = LocY = 0;
			return;
		}
	}

	/* Exit if current position is out of screen */
	if ((unsigned int)LocX >= DISP_XS || (unsigned int)LocY >= DISP_YS) return;

#if DISP_USE_DBCS
	if (Sjis1) {	/* This is sjis trailing byte */
		uint16_t bchr, rs, re;
		int ri;

		dchr = Sjis1 * 256 + chr; 
		Sjis1 = 0;
		fnt = FontD;	/* Switch to double byte font */
		i = fnt[17];	/* Number of code blocks */
		ri = 18;		/* Start of code block table */
		bchr = 0;		/* Number of chars in previous blocks */
		while (i) {		/* Find the code in the code blocks */
			rs = fnt[ri + 0] + fnt[ri + 1] * 256;	/* Start of a code block */
			re = fnt[ri + 2] + fnt[ri + 3] * 256;	/* End of a code block */
			if (dchr >= rs && dchr <= re) break;	/* Is the character in the block? */
			bchr += re - rs + 1; ri += 4; i--;		/* Next code block */
		}
		if (!i) {	/* Code not found */
			LocX += fnt[14];		/* Put a transparent character */
			return;
		}
		dchr = dchr - rs + bchr;	/* Character offset in the font area */
		fofs = 18 + fnt[17] * 4;	/* Font area start address */
	} else {
		/* Check if this is sjis leading byte */
		if (FontD && (((uint8_t)(chr - 0x81) <= 0x1E) || ((uint8_t)(chr - 0xE0) <= 0x1C))) {
			Sjis1 = chr;	/* Next is sjis trailing byte */
			return;
		}
#endif
		dchr = chr;
		fofs = 17;		/* Font area start address */
#if DISP_USE_DBCS
	}
#endif

	h = fnt[15]; w = fnt[14]; wb = (w + 7) / 8;	/* Font size: height, dot width and byte width */
	fnt += fofs + dchr * wb * h;				/* Goto start of the bitmap */

	if (LocX + w > DISP_XS) w = DISP_XS - LocX;	/* Clip right of font face at right edge */
	if (LocY + h > DISP_YS) h = DISP_YS - LocY;	/* Clip bottom of font face at bottom edge */

//	CS_LOW();	/* Select display module */
	disp_setrect(LocX, LocX + w - 1, LocY, LocY + h - 1);
	d = 0;
	do {
		wc = w; b = i = 0;
		do {
			if (!b) {		/* Get next 8 dots */
				b = 0x80;
				d = fnt[i++];
			}
			col = ChrColor;
			if (!(b & d)) col >>= 16;	/* Select color, BG or FG */
			b >>= 1;		/* Next bit */
	    LCD_RAM=col;
			//		DATA_WPX(col);	/* Put the color */
		} while (--wc);
		fnt += wb;		/* Next raster */
	} while (--h);
//	CS_HIGH();	/* Deselect display module */

	LocX += w;	/* Update current position */
}

/*******************************************************************************
* Function Name  : LCD_SetCursor
* Description    : Sets the cursor position.
* Input          : - Xpos: specifies the X position.
*                  - Ypos: specifies the Y position. 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_SetCursor(uint16_t Xpos,uint16_t Ypos)
{
	ili9341_WriteReg(0x2A);
	ili9341_WriteWord(Xpos);
	ili9341_WriteWord(Xpos);
	ili9341_WriteReg(0x2B);
	ili9341_WriteWord(Ypos);
	ili9341_WriteWord(Ypos);
}

/*******************************************************************************
* Function Name  : LCD_Delay
* Description    : Delay Time
* Input          : - nCount: Delay Time
* Output         : None
* Return         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_Delay(uint16_t nCount)
{
 volatile uint16_t TimingDelay; 
 while(nCount--)
   {
    for(TimingDelay=0;TimingDelay<20000;TimingDelay++);
   }
}

static void delay(int cnt)
{
    volatile unsigned int dl;
    while(cnt--)
    {
        for(dl=0; dl<1000; dl++);
    }
}

static __inline void ili9341_WriteReg(uint8_t reg)
{
	LCD_REG = reg;
}

static __inline void ili9341_WriteData(uint16_t data)
{
	LCD_RAM = data;
}

static __inline void ili9341_WriteWord(uint16_t data)
{
	ili9341_WriteData((data >> 8)  & 0x00FF);
	ili9341_WriteData(data & 0x00FF);
}


void LCD_Initializtion(void)
{
	uint16_t i,j;
  LCD_Configuration();
  LCD_Delay(5);  /* delay 50 ms */		
	ili9341_WriteReg(0xD3);
	DeviceCode = LCD_RAM;
	DeviceCode = LCD_RAM;
	DeviceCode = LCD_RAM;
	DeviceCode = DeviceCode<<8 | LCD_RAM;
	//printf("device code = 0x%x\r\n", DeviceCode);
	ili9341_WriteReg(0x04);
	DeviceCode = LCD_RAM;
	DeviceCode = LCD_RAM;
	DeviceCode = DeviceCode<<8 | LCD_RAM;
	//printf("identify = 0x%x\r\n", DeviceCode);
	
	
	//printf("Display Status:\r\n");
	ili9341_WriteReg(0x09);
	DeviceCode = LCD_RAM;
	//printf("-1: = 0x%x\r\n", DeviceCode);
	DeviceCode = LCD_RAM;
	//printf("-2: = 0x%x\r\n", DeviceCode);
	DeviceCode = LCD_RAM;
	//printf("-3: = 0x%x\r\n", DeviceCode);
	DeviceCode = LCD_RAM;
	//printf("-4: = 0x%x\r\n", DeviceCode);
	DeviceCode = LCD_RAM;
	//printf("-5: = 0x%x\r\n", DeviceCode);
	
	//printf("Display Pixel Format:");
	ili9341_WriteReg(0x0c);
	DeviceCode = LCD_RAM;
	DeviceCode = LCD_RAM;
	//printf(" 0x%x\r\n", DeviceCode);
	
	ili9341_WriteReg(0xcf); 
ili9341_WriteData(0x00);
ili9341_WriteData(0xc1);
ili9341_WriteData(0x30);

ili9341_WriteReg(0xed); 
ili9341_WriteData(0x64);
ili9341_WriteData(0x03);
ili9341_WriteData(0x12);
ili9341_WriteData(0x81);

ili9341_WriteReg(0xcb); 
ili9341_WriteData(0x39);
ili9341_WriteData(0x2c);
ili9341_WriteData(0x00);
ili9341_WriteData(0x34);
ili9341_WriteData(0x02);

ili9341_WriteReg(0xea); 
ili9341_WriteData(0x00);
ili9341_WriteData(0x00);

ili9341_WriteReg(0xe8); 
ili9341_WriteData(0x85);
ili9341_WriteData(0x10);
ili9341_WriteData(0x79);

ili9341_WriteReg(0xC0); //Power control
ili9341_WriteData(0x23); //VRH[5:0]

ili9341_WriteReg(0xC1); //Power control
ili9341_WriteData(0x11); //SAP[2:0];BT[3:0]

ili9341_WriteReg(0xC2);
ili9341_WriteData(0x11);

ili9341_WriteReg(0xC5); //VCM control
ili9341_WriteData(0x3d);
ili9341_WriteData(0x30);

ili9341_WriteReg(0xc7); 
ili9341_WriteData(0xaa);

ili9341_WriteReg(0x3A); 
ili9341_WriteData(0x55);

ili9341_WriteReg(0x36); // Memory Access Control
ili9341_WriteData(0x08);

ili9341_WriteReg(0xB1); // Frame Rate Control
ili9341_WriteData(0x00);
ili9341_WriteData(0x11);

ili9341_WriteReg(0xB6); // Display Function Control
ili9341_WriteData(0x0a);
ili9341_WriteData(0xa2);

ili9341_WriteReg(0xF2); // 3Gamma Function Disable
ili9341_WriteData(0x00);

ili9341_WriteReg(0xF7);
ili9341_WriteData(0x20);

ili9341_WriteReg(0xF1);
ili9341_WriteData(0x01);
ili9341_WriteData(0x30);

ili9341_WriteReg(0x26); //Gamma curve selected
ili9341_WriteData(0x01);

ili9341_WriteReg(0xE0); //Set Gamma
ili9341_WriteData(0x0f);
ili9341_WriteData(0x3f);
ili9341_WriteData(0x2f);
ili9341_WriteData(0x0c);
ili9341_WriteData(0x10);
ili9341_WriteData(0x0a);
ili9341_WriteData(0x53);
ili9341_WriteData(0xd5);
ili9341_WriteData(0x40);
ili9341_WriteData(0x0a);
ili9341_WriteData(0x13);
ili9341_WriteData(0x03);
ili9341_WriteData(0x08);
ili9341_WriteData(0x03);
ili9341_WriteData(0x00);

ili9341_WriteReg(0xE1); //Set Gamma
ili9341_WriteData(0x00);
ili9341_WriteData(0x00);
ili9341_WriteData(0x10);
ili9341_WriteData(0x03);
ili9341_WriteData(0x0f);
ili9341_WriteData(0x05);
ili9341_WriteData(0x2c);
ili9341_WriteData(0xa2);
ili9341_WriteData(0x3f);
ili9341_WriteData(0x05);
ili9341_WriteData(0x0e);
ili9341_WriteData(0x0c);
ili9341_WriteData(0x37);
ili9341_WriteData(0x3c);
ili9341_WriteData(0x0F);

  ili9341_WriteReg(LCD_SLEEP_OUT);
  LCD_Delay(200);
  ili9341_WriteReg(LCD_DISPLAY_ON);
  /* GRAM start writing */
  ili9341_WriteReg(LCD_GRAM);
	
	LCD_Delay(5);  /* delay 50 ms */	
	
//	/* Test lcd by fill rectangle */
//	ili9341_WriteReg(0x2a);
//	ili9341_WriteWord(50);
//	ili9341_WriteWord(100);
//	ili9341_WriteReg(0x2b);
//	ili9341_WriteWord(50);
//	ili9341_WriteWord(100);
//	
//	ili9341_WriteReg(LCD_GRAM);
//	for(i = 50; i<=100; i++)
//	{
//		for(j = 0; j <= 100; j++)ili9341_WriteData(0x0000);
//	}
}


/******************************************************************************
* Function Name  : LCD_GetPoint
* Description    : 获取指定座标的颜色值
* Input          : - Xpos: Row Coordinate
*                  - Xpos: Line Coordinate 
* Output         : None
* Return         : Screen Color
* Attention		 : None
*******************************************************************************/
uint16_t LCD_GetPoint(uint16_t Xpos,uint16_t Ypos)
{
    LCD_SetCursor(Xpos,Ypos);
    return ( LCD_ReadRAM() );
}

/******************************************************************************
* Function Name  : LCD_SetPoint
* Description    : 在指定座标画点
* Input          : - Xpos: Row Coordinate
*                  - Ypos: Line Coordinate 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_SetPoint(uint16_t Xpos,uint16_t Ypos,uint16_t point)
{
  if ( ( Xpos > LCD_XSIZE ) ||( Ypos>LCD_YSIZE ) ) return;
  LCD_SetCursor(Xpos,Ypos);
  LCD_WriteRAM_Prepare();
  ili9341_WriteData(point);
}

/******************************************************************************
* Function Name  : LCD_DrawPicture
* Description    : 在指定坐标范围显示一副图片
* Input          : - StartX: Row Start Coordinate 
*                  - StartY: Line Start Coordinate  
*				   - EndX: Row End Coordinate 
*				   - EndY: Line End Coordinate   
* Output         : None
* Return         : None
* Attention		 : 图片取模格式为水平扫描，16位颜色模式
*******************************************************************************/
void LCD_DrawPicture(uint16_t StartX,uint16_t StartY,uint16_t xlong,uint16_t ylong,const unsigned short *pic)
{
  uint16_t  i;
  ylong+=StartY;
  LCD_WriteReg(0x0044,(ylong<<8)|StartY);        
  LCD_WriteReg(0x0045,StartX);         
  LCD_WriteReg(0x0046,StartX+xlong-1); 
	LCD_WriteReg(0x004e,StartY); 
  LCD_WriteReg(0x004f,StartX); 	
  LCD_WriteRAM_Prepare();
  for (i=0;i<40000;i++)
  {
      LCD_WriteRAM(pic[i]);
  }
}



#if ASCII_LIB > 0 
/******************************************************************************
* Function Name  : PutChar
* Description    : 将Lcd屏上任意位置显示一个字符
* Input          : - Xpos: 水平坐标 
*                  - Ypos: 垂直坐标  
*				   - c: 显示的字符
*				   - charColor: 字符颜色   
*				   - bkColor: 背景颜色 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/

void PutChar(unsigned short Xpos,unsigned short Ypos,unsigned char c,unsigned short charColor,unsigned short bkColor)
{
  unsigned short i=0;
  unsigned short j=0;
  unsigned char buffer[16];
  unsigned char tmp_char=0;
  GetASCIICode(buffer,c);  /* 取字模数据 */
	//disp_setrect (Xpos,Xpos+7,Ypos,	Ypos+15);
  for (i=0;i<16;i++)
  {
    tmp_char=buffer[i];
    for (j=0;j<8;j++)
    {
      if ( (tmp_char >> 7-j) & 0x01 == 0x01)
        {
            //LCD_RAM= charColor; 
					  LCD_SetPoint(Xpos+j,Ypos+i,charColor);
					
        }
        else
        {
					  //LCD_RAM= bkColor;
             LCD_SetPoint(Xpos+j,Ypos+i,bkColor);  
					
        }
    }
  }
}
/******************************************************************************
* Function Name  : GUI_Text
* Description    : 在指定座标显示字符串
* Input          : - Xpos: 行座标
*                  - Ypos: 列座标 
*				   - str: 字符串
*				   - charColor: 字符颜色   
*				   - bkColor: 背景颜色 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/

void GUI_Text(uint16_t Xpos, uint16_t Ypos, uint8_t *str,uint16_t Color, uint16_t bkColor)
{
uint8_t TempChar;
 do
  {
    TempChar=*str++;  
    PutChar(Xpos,Ypos,TempChar,Color,bkColor);    
    if (Xpos<232)
    {
      Xpos+=8;
    } 
    else if (Ypos<304)
    {
      Xpos=0;
      Ypos+=16;
    }   
    else
    {
      Xpos=0;
      Ypos=0;
    }    
  }
  while (*str!=0);
}

#endif


/******************************************************************************
* Function Name  : LCD_BGR2RGB
* Description    : RRRRRGGGGGGBBBBB 改为 BBBBBGGGGGGRRRRR 格式
* Input          : - color: BRG 颜色值  
* Output         : None
* Return         : RGB 颜色值
* Attention		 : 内部函数调用
*******************************************************************************/
uint16_t LCD_BGR2RGB(uint16_t color)
{
  uint16_t  r, g, b, rgb;

  b = ( color>>0 )  & 0x1f;
  g = ( color>>5 )  & 0x3f;
  r = ( color>>11 ) & 0x1f;
 
  rgb =  (b<<11) + (g<<5) + (r<<0);

  return( rgb );
}



/*********************************************************************************************************
      END FILE
*********************************************************************************************************/

