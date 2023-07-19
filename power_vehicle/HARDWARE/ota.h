#ifndef __OTA_H__
#define	__OTA_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "common_def.h"

#define COMM_DATA_BUFF_SIZE 2048

#define COMM_OTA_SYN_HEAD 	0x6688
#define COMM_OTA_SYN_HEAD1 	0x88
#define COMM_OTA_SYN_HEAD2 	0x66

#define COMM_OTA_OFFSET(member) (u32)&(((struct ota_protocol *)0)->member)

enum main_command {
	MAIN_COMMAND_START = 0,
	MAIN_COMMAND_OTA = MAIN_COMMAND_START,
	MAIN_COMMAND_END,
};

enum ota_sub_command {
	OTA_SUB_COMMAND_START = 0,
	OTA_SYN_HEAD = OTA_SUB_COMMAND_START,
	OTA_PROGRAM_FLASH,
	OTA_CHECKSUM_FLASH,
	OTA_RUN_BOOTLOADER,
	OTA_RUN_APPLICATION,
	OTA_SUB_COMMAND_END,
};

__packed struct ota_protocol {
	UINT16 synHead;							// 0x6688
	UINT32 checkSum;							// check sum
	UINT8 	mainCommand;					// main command value
	UINT8 	subCommand;						// sub command value
	UINT16 dataLen;
	UINT8 	data[COMM_DATA_BUFF_SIZE];
};

__packed struct comm_ota_data {
	bool receiveFinish;
	UINT8 state;
	UINT16 rxCnt;
	struct ota_protocol *otaData;
};

__packed struct main_command_process {
	enum main_command mainCommand;
	INT32 (*mainCommandProcess)(struct ota_protocol *);
};

struct comm_ota_data *GetCommunicationData(void);
void CommunicationProcess(void);
void CommOtaIrqHandler(UINT8 ch);

#ifdef __cplusplus
}
#endif

#endif 

