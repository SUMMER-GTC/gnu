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
#define PAGE_LOG						0		/* home */
#define PAGE_HOME						1		/* main */

#define KEY_RETURN_POWER_INC_DEC	0x0001
#define KEY_RETURN_START					0x0002

#define DATA_POWER	0x0010
#define DATA_RPM		0x0011
#define DATA_TIME		0x0012

#define TEXT_SOFTWARE_VER		0x0020

#define VAR_ICON_SPO2_L	0x0030
#define VAR_ICON_SPO2_M	0x0031
#define VAR_ICON_SPO2_H	0x0032

#define VAR_ICON_VO2_L	0x0033
#define VAR_ICON_VO2_M	0x0034
#define VAR_ICON_VO2_H	0x0035

#define VAR_ICON_VCO2_L	0x0036
#define VAR_ICON_VCO2_M	0x0037
#define VAR_ICON_VCO2_H	0x0038

#define VAR_ICON_HEART_RATE_L	0x0039
#define VAR_ICON_HEART_RATE_M	0x003A
#define VAR_ICON_HEART_RATE_H	0x003B

#define VAR_ICON_LBP_L	0x003C
#define VAR_ICON_LBP_M	0x003D
#define VAR_ICON_LBP_H	0x003E

#define VAR_ICON_HBP_L	0x003F
#define VAR_ICON_HBP_M	0x0040
#define VAR_ICON_HBP_H	0x0041

#define DATA_RPM_SP_COLOR	0x6003
#define DATA_RPM_RED			0xF800
#define DATA_RPM_GREEN		0x07E8

#define VAR_ICON_NUM_0	0
#define VAR_ICON_NUM_1	1
#define VAR_ICON_NUM_2	2
#define VAR_ICON_NUM_3	3
#define VAR_ICON_NUM_4	4
#define VAR_ICON_NUM_5	5
#define VAR_ICON_NUM_6	6
#define VAR_ICON_NUM_7	7
#define VAR_ICON_NUM_8	8
#define VAR_ICON_NUM_9	9
#define VAR_ICON_NUM___	10
#define VAR_ICON_NUM_NC	11


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


