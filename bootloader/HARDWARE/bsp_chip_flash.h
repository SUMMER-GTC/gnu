#ifndef __BSP_CHIP_FLASH_H__
#define __BSP_CHIP_FLASH_H__
#include "stm32f10x.h"
#include <stdbool.h>

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

void ChipFlashEraseAppRom(void);
s32 ChipFlashPageWrite(u8 *dataBuff, u32 chipAddr, u32 dataLen, bool eraseFlag);
s32 ChipFlashPagRead(u8 *dataBuff, u32 chipAddr, u32 dataLen);
u32 ChipFlashCheckSum(u32 pageAddress, u32 len);
struct sys_config_opt* GetSysConfigOpt(void);
	
#endif

