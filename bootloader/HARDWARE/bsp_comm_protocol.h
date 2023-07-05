#ifndef __BSP_COMM_PROTOCOL_H__
#define	__BSP_COMM_PROTOCOL_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "bsp_common_define.h"

#define COMM_DATA_BUFF_SIZE 2048

#define COMM_OTA_SYN_HEAD 	0x6688
#define COMM_OTA_SYN_HEAD1 	0x88
#define COMM_OTA_SYN_HEAD2 	0x66

#define COMM_OTA_OFFSET(member) (u32)&(((struct ota_protocol *)0)->member)

enum {
	COMM_IDLE_STATE = 0,
	COMM_SYN_HEAD1_STATE = 1,
	COMM_SYN_HEAD2_STATE,
	COMM_DATA_LEN_STATE
};

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
	u16 synHead;							// 0x6688
	u32 checkSum;							// check sum
	u8 	mainCommand;					// main command value
	u8 	subCommand;						// sub command value
	u16 dataLen;
	u8 	data[COMM_DATA_BUFF_SIZE];
};

__packed struct comm_ota_data {
	bool receiveFinish;
	u8 state;
	u16 rxCnt;
	struct ota_protocol *otaData;
};

__packed struct main_command_process {
	enum main_command mainCommand;
	s32 (*mainCommandProcess)(struct ota_protocol *);
};

struct comm_ota_data *GetCommunicationData(void);
void CommunicationProcess(void);
void CommOtaIrqHandler(u8 ch);

#ifdef __cplusplus
}
#endif

#endif 
/* __BSP_COM_PROTOCOL_H__ */
