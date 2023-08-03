#include "stm32f4xx_flash.h"
#include "chip_flash.h"
#include "common_def.h"

static const unsigned char g_appDeviceNameBuf[] __attribute__((__used__, __section__(".device_name"))) = "power vehicle";
static const unsigned char g_appHardwareVersionBuf[] __attribute__((__used__, __section__(".hardware_ver"))) = "ver.1.0";

void ChipFlashEraseAppRom(void)
{
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGAERR | FLASH_FLAG_WRPERR);

	for (u16 sector = APPLICATION_START_SECTOR; sector <= APPLICATION_END_SECTOR; sector += APPLICATION_SECTOR_OFFSET) {
  		FLASH_EraseSector(sector, VoltageRange_3);
	}

	FLASH_Lock();
}


/*
* @brief  chip flash page write, write 1 words(4 bytes) one time, 
*					and it will erase flash before write
* @param  
* @retval 
*/
s32 ChipFlashPageWrite(u8 *dataBuff, u32 chipAddr, u32 dataLen, bool eraseSysConfSectorFlag)
{
	u32 *p = (u32*)dataBuff;
	u32 data;
	u32 writeAddr = chipAddr;
	/* Enable Prefetch Buffer */
	FLASH_PrefetchBufferCmd(ENABLE);
	/* Flash 2 wait state */
	FLASH_SetLatency(FLASH_Latency_5);
 	//FLASH_EraseOptionBytes();
	//FLASH_ProgramOptionByteData(0x1FFFF804, 0);
	//FLASH_EnableWriteProtection(FLASH_WRProt_AllPages);
	
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGAERR | FLASH_FLAG_WRPERR); 
	if (eraseSysConfSectorFlag) {
  	FLASH_EraseSector(SYS_CONFIG_SECTOR, VoltageRange_3);
	}
	
	for(u32 i = 0; i < dataLen / sizeof(u32); i++)
	{
		data = *p++;
		(void)FLASH_ProgramWord(writeAddr , data);  
		writeAddr += sizeof(u32);
	}
	FLASH_Lock();	
	return SUCC;
}

/*
* @brief  chip flash page read, read 1 words(4 bytes) one operation
* @param  
* @retval 
*/
s32 ChipFlashPagRead(u8 *dataBuff, u32 chipAddr, u32 dataLen)
{
	u32 *p = (u32*)dataBuff;
	u32 data;
	u32 readAddr = chipAddr;
	
	for(u32 i = 0; i < dataLen / sizeof(u32); i++)
	{
		data = *( u32*) readAddr;
		readAddr += 4;
		*p++ = data;
	}
	return SUCC;
}

/*
* @brief  
* @param  
* @retval 
*/
u32 ChipFlashCheckSum(u32 pageAddress, u32 len)
{
	u32 checkSum=0;
	
 	for(u32 i = 0; i < len; i++) {
		checkSum += (*(u8*)(pageAddress + i));
	}

	return checkSum;
}

static struct sys_config g_sysConfig;

static void ReadSysConfigFromFlash(void)
{
	u8 *readBuff = (u8*)&g_sysConfig;
	ChipFlashPagRead(readBuff, SYS_CONFIG_ADDRESS, sizeof(g_sysConfig));
}

static void WriteSysConfigToFlash(void)
{
	u8 *writeBuff = (u8 *)&g_sysConfig;
	__disable_irq();
	ChipFlashPageWrite(writeBuff, SYS_CONFIG_ADDRESS, sizeof(g_sysConfig), true);
	__enable_irq();
}

/*
* @brief  
* @param  
* @retval 
*/
static void SysConfigInit(void)
{

}

static struct sys_config_opt g_sysConfigOpt = {
	.sysConfig = &g_sysConfig,
	.Init = SysConfigInit,
	.Read = ReadSysConfigFromFlash,	
	.Write = WriteSysConfigToFlash
};

struct sys_config_opt* GetSysConfigOpt(void)
{
	return &g_sysConfigOpt;
}





