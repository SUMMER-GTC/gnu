#ifndef __USART_SCREEN_H__
#define __USART_SCREEN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common_def.h"

#define COMM_DGUS_BUFF_SIZE 				64

#define COMM_DGUS_SYN_HEAD1					0x5A
#define COMM_DGUS_SYN_HEAD2					0xA5

#define COMM_DGUS_OFFSET(member) (UINT32)&(((struct dgus_protocol *)0)->member)

#define DGUS_CMD_REG_WR		0x80	/* reg write */
#define DGUS_CMD_REG_RD		0x81	/* reg read */
#define DGUS_CMD_MEM_WR		0x82	/* memory write */
#define DGUS_CMD_MEM_RD		0x83	/* memory read */
#define DGUS_CMD_DATABUF	0x84	/* curve buffer */

#define DGUS_CMD_RD_PAGE  0X03  // read page

#define DGUS_FRAME_HEADER0	0x5A
#define DGUS_FRAME_HEADER1	0xA5

#define DGUS_MEMORD_CMD			3
#define DGUS_MEMORD_ADDH		4
#define DGUS_MEMORD_ADDL		5
#define DGUS_MEMORD_RD_LEN	6 // how many words data
#define DGUS_MEMORD_DATAH		7
#define DGUS_MEMORD_DATAL		8

/* page define */
#define PAGE_LOG						0		/* log */
#define PAGE_STANDALONE			1		/* standalone */
#define PAGE_HEART_LUNG			2		/* heart lung */
#define PAGE_PARAMETER_CONF 3

#define KEY_RETURN_POWER_INC_DEC	0x0001
#define KEY_RETURN_START					0x0002

#define DATA_POWER	0x0010
#define DATA_RPM		0x0011
#define DATA_TIME		0x0012
#define DATA_SPO2		0x0013
#define DATA_VO2		0x0014
#define DATA_VCO2		0x0015
#define DATA_HR			0x0016
#define DATA_LBP		0x0017
#define DATA_HBP		0x0018

#define TEXT_SOFTWARE_VER		0x0020

#define VAR_ICON_START_STOP	0x0030
#define START_ICON 0
#define STOP_ICON 1

// parameter config
#define DATA_KCOEF				0x0040
#define DATA_KP						0x0041
#define DATA_KI						0x0042
#define DATA_KD						0x0043
#define KEY_KCOEF_INC_DEC	0x0044
#define KEY_KP_INC_DEC		0x0045
#define KEY_KI_INC_DEC		0x0046
#define KEY_KD_INC_DEC		0x0047
#define KEY_RETURN_ENTRY_PARAMETER_CONF	0x0048 // press 6 times entry
#define KEY_RETURN_EXIT_PARAMETER_CONF	0x0049
#define KEY_RETURN_CALIBRATION					0x004A
#define TEXT_CALIBRATION_DIS 						0x0050

#define DATA_RPM_SP_COLOR	0x6003
#define DATA_RPM_RED			0xF800
#define DATA_RPM_GREEN		0x07E8


#define GET_UINT16_H(wValue) (UINT8)((wValue & 0xff00) >> 8)
#define GET_UINT16_L(wValue) (UINT8)((wValue) & 0x00ff)

typedef enum {
	DGUS_DATA_W_CMD = 0,
	DGUS_DATA_R_CMD,
	DGUS_TEXT_W_CMD,
	DGUS_TEXT_R_CMD,
	DGUS_PAGE_W_CMD,
	DGUS_PAGE_R_CMD,
	UART_SCREEN_START_TIMER_CMD,
} UART_SCREEN_CMD;

typedef enum {
	UART_SCREEN_LOG_DELAY_DATA = 0,
	UART_SCREEN_TIMER_DATA,
	UART_SCREEN_DGUS_DATA,
	UART_SCREEN_OTHER_DATA,
} UART_SCREEN_DATA_TYPE;

struct dgus_addr_data {
	UINT16 addr;
	UINT16 data[8];
} __packed;

struct dgus_protocol {
	UINT16 	synHead;			// 0x5AA5
	UINT8 	dataLen;
	UINT8 	command;
	UINT8 	data[COMM_DGUS_BUFF_SIZE];
} __packed;

struct comm_dgus_data {
	UINT8 state;
	UINT8 rxCnt;
	struct dgus_protocol *dgusData;
} __packed;

struct uart_screen {
	UINT8 dataType;
	UINT8 *data;
} __packed;

#ifdef __cplusplus
}
#endif


#endif


