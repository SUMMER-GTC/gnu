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
#define PAGE_HOME						0		/* home */
#define PAGE_STANDALONE			1		/* standalone version */
#define PAGE_HEART_LUNG			2		/* heart lung version */
#define PAGE_VERSION				3		/* page version */

#define DATA_RPM			0x0001
#define DATA_POWER		0x0002
#define DATA_D1				0x0003
#define DATA_D2				0x0004
#define DATA_D3				0x0005
#define DATA_D4				0x0006
#define DATA_D5				0x0007

#define KEY_RETURE_STANDALONE	0x0010
#define KEY_RETURE_HEART_LUNG	0x0011
#define KEY_RETURN_VER_QUERY	0x0012
#define KEY_RETURN_HOME				0x0013

#define TEXT_SOFTWARE_VER		0x0020
#define TEXT_HARDWARE_VER		0x0028

#define GET_UINT16_H(wValue) (UINT8)((wValue & 0xff00) >> 8)
#define GET_UINT16_L(wValue) (UINT8)((wValue) & 0x00ff)

typedef enum {
	DGUS_DATA_W_CMD = 0,
	DGUS_DATA_R_CMD,
	DGUS_TEXT_W_CMD,
	DGUS_TEXT_R_CMD,
	DGUS_PAGE_W_CMD,
	DGUS_PAGE_R_CMD,
} DGUS_CMD;

struct dgus_addr_data {
	UINT16 addr;
	UINT8 data[16];
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

#ifdef __cplusplus
}
#endif


#endif


