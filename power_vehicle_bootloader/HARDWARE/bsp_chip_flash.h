#ifndef __BSP_CHIP_FLASH_H__
#define __BSP_CHIP_FLASH_H__
#include "stm32f4xx.h" 
#include <stdbool.h>

/*
	FLASH_Sector_0 bootloader(16KB)
	FLASH_Sector_1 system config parameters(16KB)
	FLASH_Sector_2 - FLASH_Sector_11 application(992KB)
*/
#define BOOTLOADER_ADDRESS		          0x08000000	// 16KB(0x4000) used by bootloader
#define SYS_CONFIG_ADDRESS		          0x08004000	// 2KB size
#define DEVICE_NAME_BUFF_ADDRESS				0x08004840  // 64B size
#define HARDWARE_VERSION_BUFF_ADDRESS		0x08004880  // 64B size
#define APPLICATION_ADDRESS							0x08008000	// 992KB used by application			

#define APPLICATION_START_SECTOR		 		FLASH_Sector_2	// application code address
#define APPLICATION_END_SECTOR		 			FLASH_Sector_11
#define APPLICATION_SECTOR_OFFSET				((uint16_t)0x0008)
#define SYS_CONFIG_SECTOR		          	FLASH_Sector_1	// 16KB size

#define FLASH_SIZE_1K 1024
#define FLASH_SIZE_2K 2048

#define FIRST_USE_ID  0x55AABBCD

#define APPLICATION_SP_MASK				0x2FFC0000
#define APPLICATION_SP_OK					0x20000000

#define ROM_SIZE_8K	8192

struct flash_sector {
	u16 sectorEndAddr;
	u16 sectorNum;
} __packed;

void ChipFlashEraseAppRom(u32 codeEndAddr);
s32 ChipFlashPageWrite(u8 *dataBuff, u32 chipAddr, u32 dataLen, bool eraseFlag);
s32 ChipFlashPagRead(u8 *dataBuff, u32 chipAddr, u32 dataLen);
u32 ChipFlashCheckSum(u32 pageAddress, u32 len);
struct sys_config_opt* GetSysConfigOpt(void);
	
#endif

