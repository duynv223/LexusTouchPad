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
#include "touchpad.h"
#include "ps2_io.h"

/******************************************************************************
Private Definitions
******************************************************************************/


#define CMD_RESET						0xFF
/*  des: Perform a software reset and recalibration
	par: none
	res: 
*/

#define CMD_RESEND						0xFE
/*  des: re transmit the last packet
	par: none
	res: <ACK>
*/

#define CMD_SET_DEFAULT					0xF6
/*  des: Restore conditions to the initial power-up state
	par: none
	res: <ACK>
*/

#define CMD_DISABLE						0xF5
/*  cmd: Disable Stream mode reporting of motion data packets
	par: none
	res: <ACK>
*/

#define CMD_ENABLE						0xF4
/*  cmd: Begin sending motion data packets if in Stream mode
	par: none
	res: <ACK>
*/

#define CMD_SET_SAMPLE_RATE				0xF3
/*  cmd: sets the PS/2 “sample rate” parameter to the specified value in samples per second
	     Synaptic touch pad only support 40/80 sample rates
	par: <Sample Rate>
	res: <ACK>
*/
#define CMD_READ_DEVICE_TYPE			0xF2
/*  cmd: Read divice id
	par: none
	res: <ACK> <00>
*/

#define CMD_SET_REMOTE_MODE				0xF0
/*  cmd: distinct from the default
         Stream mode. In Remote mode, the device sends motion data packets only
         in response to a Read Data ($EB) command
	par: none
	res: <ACK>
*/

#define CMD_SET_WRAP_MODE				0xEE
/*  cmd: Set echo mode. All byte send to device will echo back except 0xFE
	par: none
	res: <ACK>
*/

#define CMD_RESET_WRAP_MODE				0xEC
/*  cmd: exit echo mode, it will disable stream mode.
	par: none
	res: <ACK>
*/

#define CMD_READ_DATA					0xEB
/*  cmd: Get motion data packet. It the only way to get data in remote mode
	par: none
	res: <ACK> <3 or 6 motion data packet>
*/

#define CMD_SET_STREAM_MODE				0xEA
/*  cmd: Switch to stram mode. The touch pad will send motion packet to host every
         time finger motion or button event occur and data reporting is enabled
	par: none
	res: <ACK>
*/

#define CMD_STATUS_REQ					0xE9
/*  cmd: Request status
	par: none
	res: <ACK> <3 byte status>
	| bit 7 | bit 6 | bit 5 | bit 4 | bit 3 | bit 2 | bit 1 | bit 0 |
	+-------+-------+-------+-------+-------+-------+-------+-------+
  0 |   0   |Remote |Enable |Scaling|   0   |   L   |   M   |   R   |
	+-------+-------+-------+-------+-------+-------+-------+-------+
  1 |   0   |   0   |   0   |   0   |   0   |   0   |   Resolution  |
	+-------+-------+-------+-------+-------+-------+-------+-------+
  2 |                          Sample Rate                          |
	+---------------------------------------------------------------+
*/

#define CMD_SET_RESOLUTION				0xE8
/*  cmd: Set resolution
	par: <solution>
		00: 1 count/mm
		01: 2 count/mm
		02: 4 count/mm
		03: 8 count/mm
	res: <ACK>
*/

#define CMD_SET_SCALING_2_1				0xE7
/*  cmd: Set non-linear scaling
         it not actually affect TouchPad data reporting
	par: none
	res: <ACK>
*/

#define CMD_SET_SCALING_1_1				0xE6
/*  cmd: Clear non-linear scaling
	par: none
	res: <ACK>
*/

/* Special commands for synaptic touch pad */
/*  Information queries 
	Sequence <8 bit cmd send by E8> <E9>
*/
#define CMD_SP_TOUCHPAD_ID				0x00
/*  cmd: Read touchpad ID
	par: none
	res: <ACK> <3 bytes info>
	
	+-------+-------+-------+-------+-------+-------+-------+-------+
	| bit 7 | bit 6 | bit 5 | bit 4 | bit 3 | bit 2 | bit 1 | bit 0 |
	+-------+-------+-------+-------+-------+-------+-------+-------+
  0 |                           infoMinor                           |
	+-------+-------+-------+-------+-------+-------+-------+-------+
  1 |   0   |   1   |   0   |   0   |   0   |   1   |   1   |   1   | <for synaptic>
	+-------+-------+-------+-------+-------+-------+-------+-------+
  2 |         infoModelCode         |           infoMajor			|
	+---------------------------------------------------------------+
	
*/

#define CMD_SP_READ_TOUCHPAD_MODE		0x01
/*  cmd: Read touchpad mode
	par: none
	res: <ACK> <3B> <47> <mode byte>
*/

#define CMD_SP_READ_CAPABILITY			0x02
/*  cmd: Read touchpad capabilities
	par: none
	res: <ACK> <cap 15-8> <47> <cap 7-0>
*/

#define CMD_SP_READ_MODEL_ID			0x03
/*  cmd: Read model id
	par: none
	res: <ACK> <model 23-16> <model 15-8> <model 7-0>
*/

#define CMD_SP_READ_SERIAL_PREFIX		0x04
/*  cmd: Read serial number prefix
	par: none
	res: <ACK> <serial 31-24> <serial 35-32>
*/

#define CMD_SP_READ_SERIAL_SUFFIX		0x05
/*  cmd: Read serial number suffix
	par: none
	res: <ACK> <serial 23-16> <serial 15-8> <serial 7-0>
*/

#define CMD_SP_READ_RESOLUTION			0x05
/*  cmd: Read X, Y cordinate resolution
	par: none
	res: <ACK> <infoXupmm> <8x> <infoYupmm>
*/

#define CMD_SP_SET_MODE					0x05
/*  cmd: Read X, Y cordinate resolution
	par: none
	res: <ACK> <infoXupmm> <8x> <infoYupmm>
*/


/*  Mode setting sequence
	Sequence: <8 bit cmd send by E8> <F3> <14>
  +-------+-------+-------+-------+----------+-------+--------+-------+
  | bit 7 | bit 6 | bit 5 | bit 4 | bit 3    | bit 2 | bit 1  | bit 0 |
  +-------+-------+-------+-------+----------+-------+--------+-------+
  |  Abs  |  Rate |   -   |   -   |Baud/Sleep|DisGest|PackSize| Wmode |
  +-------+-------+-------+-------+----------+-------+--------+-------+
  
*/

#define TOUCHPAD_BEZEL_LIMIT_X_MIN		1472
#define TOUCHPAD_BEZEL_LIMIT_X_MAX		5472

#define TOUCHPAD_BEZEL_LIMIT_Y_MIN		1408
#define TOUCHPAD_BEZEL_LIMIT_Y_MAX		4448

#define TOUCHPAD_BEZEL_MRRGIN_X_MIN		1632
#define TOUCHPAD_BEZEL_MRRGIN_X_MAX		5312

#define TOUCHPAD_BEZEL_MRRGIN_Y_MIN		1568
#define TOUCHPAD_BEZEL_MRRGIN_Y_MAX		4288


typedef struct {
	uint8_t	Absolute;
	/*	0: Relative mode
		1: Absolute mode
	*/
	uint8_t	Rate;
	/*	0: 40 package/s stream rate
		1: 80 package/s stream rate
	*/

	uint8_t	Sleep;
	/*	0: disable sleep mode
		1: enable sleep mode
	*/
	
	uint8_t	DisGest;
	/*	0: disable tap and drag
		1: enable tap and drag
	*/
	
	uint8_t	PackSize;
	/*	0: six bytes absolute package format
		1: seven or eight bytes absolute package format
	*/
	
	uint8_t	WMode;
	/*	0: normal absolute package
		1: enhanced absolute package
	*/
}Touchpad_ModeTypedef;
	

/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/

/******************************************************************************
Private global variables
******************************************************************************/
void (*gp_MotionPkgReportCb)(Touchpad_MotionPackageTypedef pkg);

/******************************************************************************
Private function prototypes
******************************************************************************/
void Touchpad_SectionDoneHandler(const uint8_t * bytes, uint8_t n);

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
void Touchpad_SendArg(uint8_t arg)
{
	uint8_t i;
	for(i = 0; i < 4; ++i)
		{
			PS2_SendByteAndWaitAck(0xE8);
			PS2_SendByteAndWaitAck((arg >> (6-2*i)) & 0x03);
		}
}

void Touchpad_SetMode(Touchpad_ModeTypedef mode)
{
	uint8_t modeArg = 0;
	modeArg = 
		((mode.Absolute & 0x01) << 7) |
		((mode.Rate     & 0x01) << 6) |
		((mode.Sleep    & 0x01) << 3) |
		((mode.DisGest  & 0x01) << 2) |
		((mode.PackSize & 0x01) << 1) |
		((mode.WMode    & 0x01) << 0);

	Touchpad_SendArg(modeArg);
	PS2_SendByteAndWaitAck(0xF3);
	PS2_SendByteAndWaitAck(0X14);
}


void Touchpad_Init(void)
{
	Touchpad_ModeTypedef mode;
	
	PS2_Init();

	/* Reset */
	PS2_SendByteAndWaitAck(CMD_RESET);
	
	mode.Absolute = 1;
	mode.Rate = 0;
	mode.Sleep = 0;
	mode.DisGest = 0;
	mode.PackSize = 0;
	mode.WMode = 0;
	
	Touchpad_SetMode(mode);

	PS2_SendByteAndWaitAck(CMD_ENABLE);

	PS2_RegistSectionDoneCb(Touchpad_SectionDoneHandler);
}

void Touchpad_SectionDoneHandler(const uint8_t * bytes, uint8_t n)
{
	Touchpad_MotionPackageTypedef pkg;
	/* Confirm motion package */
	if( (n == 6) &&
		((bytes[0] & 0xC8) == 0x80) &&
		((bytes[3] & 0xC0) == 0xC0)
	)
		{
			pkg.Finger  = (bytes[0] & (1<<5)) ? true:false;
			pkg.Gesture = (bytes[0] & (1<<2)) ? true:false;
			pkg.Righ    = (bytes[0] & (1<<1)) ? true:false;
			pkg.Left    = (bytes[0] & (1<<0)) ? true:false;

			pkg.ZPressure = bytes[3];
			pkg.XPos = ((bytes[1] &0x0f) << 8) | ((bytes[3] & 0x10) << (12 - 4)) | bytes[4];
			pkg.YPos = ((bytes[1] &0xf0) << (8 - 4)) | ((bytes[3] & 0x20) << (12 - 5)) | bytes[5];
			if(gp_MotionPkgReportCb)
				gp_MotionPkgReportCb(pkg);
		}
}

void Touchpad_RegistMotionPkgReportHandler(void (*f)(Touchpad_MotionPackageTypedef pkg))
{
	gp_MotionPkgReportCb = f;
}

//void Touchpad_SetReportRate();




/******************* (C) COPYRIGHT DUYNV  **********************END OF FILE****/
