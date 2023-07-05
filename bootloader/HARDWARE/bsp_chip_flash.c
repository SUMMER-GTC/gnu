#include "stm32f10x_flash.h"
#include "bsp_chip_flash.h"
#include "bsp_common_define.h"

void ChipFlashReadOutProtection(void)
{
	if (FLASH_GetReadOutProtectionStatus() != SET) {
		FLASH_Unlock();
		FLASH_ReadOutProtection(ENABLE);
		FLASH_Lock();
	}
}

void ChipFlashEraseAppRom(void)
{
	FLASH_UnlockBank1();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 

	for (u32 addr = APPLICATION_ADDRESS; addr < SYS_CONFIG_ADDRESS; addr += FLASH_SIZE_2K) {
  	(void)FLASH_ErasePage(addr);
	}

	FLASH_LockBank1();
}


/*
* @brief  chip flash page write, write 1 words(4 bytes) one time, 
*					and it will erase flash before write
* @param  
* @retval 
*/
s32 ChipFlashPageWrite(u8 *dataBuff, u32 chipAddr, u32 dataLen, bool eraseFlag)
{
	u32 *p = (u32*)dataBuff;
	u32 data;
	u32 writeAddr = chipAddr;
	/* Enable Prefetch Buffer */
	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
	/* Flash 2 wait state */
	FLASH_SetLatency(FLASH_Latency_2);
 	FLASH_EraseOptionBytes();
	//FLASH_ProgramOptionByteData(0x1FFFF804, 0);
	FLASH_EnableWriteProtection(FLASH_WRProt_AllPages);
	
	FLASH_UnlockBank1();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 
	if (eraseFlag) {
  	(void)FLASH_ErasePage(chipAddr);
	}
	
	for(u32 i = 0; i < dataLen / sizeof(u32); i++)
	{
		data = *p++;
		(void)FLASH_ProgramWord(writeAddr , data);  
		writeAddr += sizeof(u32);
	}
	FLASH_LockBank1();	
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
	ChipFlashPageWrite(writeBuff, SYS_CONFIG_ADDRESS, sizeof(g_sysConfig), true);
}

/*
* @brief  
* @param  
* @retval 
*/
static void SysConfigInit(void)
{
	ReadSysConfigFromFlash();
	if (g_sysConfig.firstUseId != FIRST_USE_ID) {
		g_sysConfig.firstUseId = FIRST_USE_ID;
		g_sysConfig.hardWareErrCnt = 0;
		g_sysConfig.deviceNameErrCnt = 0;
		g_sysConfig.romCheckErrCnt = 0;
		WriteSysConfigToFlash();
	}
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





