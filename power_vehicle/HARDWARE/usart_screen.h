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


