#ifndef __USART_SCREEN_H__
#define __USART_SCREEN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common_def.h"

#define COMM_DGUS_BUFF_SIZE 				64
#define COMM_CODE_UPDATE_BUFF_SIZE	1

#define COMM_DGUS_SYN_HEAD1					0x5A
#define COMM_DGUS_SYN_HEAD2					0xA5

#define COMM_CODE_UPDATE_SYN_HEAD1	0x66
#define COMM_CODE_UPDATE_SYN_HEAD2	0x88

#define MAIN_CMD_UPDATE_CODE 				0x01
#define SUB_CMD_ENTRY_BOOTLOADER		0x01

#define COMM_DGUS_OFFSET(member) (UINT32)&(((struct dgus_protocol *)0)->member)
#define COMM_CODE_UPDATE_OFFSET(member) (UINT32)&(((struct code_update_protocol *)0)->member)


__packed struct dgus_protocol {
	UINT16 	synHead;			// 0x5AA5
	UINT8 	dataLen;
	UINT8 	command;
	UINT8 	data[COMM_DGUS_BUFF_SIZE];
};

__packed struct code_update_protocol {
	UINT16 	synHead;			// 0x6688
	UINT32 	checkSum;			// check sum
	UINT8		mainCommand;	// main command value
	UINT8 	subCommand;		// sub command value
	UINT16 	dataLen;
	UINT8 	data[COMM_CODE_UPDATE_BUFF_SIZE];
};

__packed struct comm_dgus_data {
	UINT8 state;
	UINT8 rxCnt;
	struct dgus_protocol *dgusData;
};

__packed struct comm_code_update_data {
	UINT8 state;
	UINT8 rxCnt;
	struct code_update_protocol *codeUpdateData;
};

#ifdef __cplusplus
}
#endif


#endif


