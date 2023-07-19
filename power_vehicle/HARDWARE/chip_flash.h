#ifndef __CHIP_FLASH_H__
#define __CHIP_FLASH_H__
#include "common_def.h"

/*
	Flash变量相关定义, 以STM32F103VCT6为例, Flash大小为256K
*/
#define BOOTLOADER_ADDRESS		          0x08000000	// 16KB(0x4000) used by bootloader
#define APPLICATION_ADDRESS		          0x08004000	// application code address
#define DEVICE_NAME_BUFF_ADDRESS				0x0803F040  // 64B size
#define HARDWARE_VERSION_BUFF_ADDRESS		0x0803F080  // 64B size
#define SYS_CONFIG_ADDRESS		          0x0803F800	// 2KB size, address at the 254KB(0x3F800)

#define FLASH_SIZE_1K 1024
#define FLASH_SIZE_2K 2048

#define FIRST_USE_ID  0x55AABBCD

#define APPLICATION_SP_MASK				0x2FFF0000
#define APPLICATION_SP_OK					0x20000000

#define ROM_SIZE_8K	8192

__packed struct sys_config {
	UINT32 firstUseId;
	UINT32 otaState;
	UINT32 hardWareErrCnt;
	UINT32 deviceNameErrCnt;
	UINT32 romCheckErrCnt;
};

__packed struct sys_config_opt {
	struct sys_config *sysConfig;
	void (*Init)(void);
	void (*Read)(void);
	void (*Write)(void);
};

void ChipFlashEraseAppRom(void);
INT32 ChipFlashPageWrite(UINT8 *dataBuff, UINT32 chipAddr, UINT32 dataLen, bool eraseFlag);
INT32 ChipFlashPagRead(UINT8 *dataBuff, UINT32 chipAddr, UINT32 dataLen);
UINT32 ChipFlashCheckSum(UINT32 pageAddress, UINT32 len);
struct sys_config_opt* GetSysConfigOpt(void);
	
#endif

