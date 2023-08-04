#ifndef __CHIP_FLASH_H__
#define __CHIP_FLASH_H__
#include "stm32f4xx.h" 
#include <stdbool.h>
#include "common_def.h"

/*
	FLASH_Sector_0 bootloader(16KB)
	FLASH_Sector_1 system config parameters(16KB)
	FLASH_Sector_2 boot parameters(16KB)
	FLASH_Sector_3 - FLASH_Sector_11 application(976KB)
*/
#define BOOTLOADER_ADDRESS		          0x08000000	// 16KB(0x4000) used by bootloader
#define SYS_CONFIG_ADDRESS		          0x08004000	// 16KB size
#define DEVICE_NAME_BUFF_ADDRESS				0x08008000  // 32B size
#define HARDWARE_VERSION_BUFF_ADDRESS		0x08008020  // 32B size
#define APPLICATION_ADDRESS							0x0800C000	// 976KB used by application			

#define APPLICATION_START_SECTOR		 		FLASH_Sector_3	// application code address
#define APPLICATION_END_SECTOR		 			FLASH_Sector_11
#define APPLICATION_SECTOR_OFFSET				((uint16_t)0x0008)
#define SYS_CONFIG_SECTOR		          	FLASH_Sector_1	// 16KB size

#define FLASH_SIZE_1K 1024
#define FLASH_SIZE_2K 2048

#define FIRST_USE_ID  0x55AABBCD

#define APPLICATION_SP_MASK				0x2FFC0000
#define APPLICATION_SP_OK					0x20000000

#define ROM_SIZE_8K	8192

void ChipFlashEraseAppRom(void);
s32 ChipFlashPageWrite(u8 *dataBuff, u32 chipAddr, u32 dataLen, bool eraseFlag);
s32 ChipFlashPagRead(u8 *dataBuff, u32 chipAddr, u32 dataLen);
u32 ChipFlashCheckSum(u32 pageAddress, u32 len);
struct sys_config_opt* GetSysConfigOpt(void);

extern const char g_appHardwareVersionBuf[];
extern const char g_appSoftwareVersionBuf[];

#endif

