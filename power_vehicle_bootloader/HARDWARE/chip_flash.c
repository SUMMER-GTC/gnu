#include "stm32f4xx_flash.h"
#include "chip_flash.h"

void ChipFlashReadOutProtection(void)
{
	// if (FLASH_GetReadOutProtectionStatus() != SET) {
	// 	FLASH_Unlock();
	// 	FLASH_ReadOutProtection(ENABLE);
	// 	FLASH_Lock();
	// }
}

void ChipFlashEraseAppRom(void)
{
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR | FLASH_FLAG_WRPERR); 

	for (UINT32 addr = APPLICATION_ADDRESS; addr < SYS_CONFIG_ADDRESS; addr += FLASH_SIZE_2K) {
  	//(void)FLASH_ErasePage(addr);
	}

	FLASH_Lock();
}


/*
* @brief  chip flash page write, write 1 words(4 bytes) one time, 
*					and it will erase flash before write
* @param  
* @retval 
*/
INT32 ChipFlashPageWrite(UINT8 *dataBuff, UINT32 chipAddr, UINT32 dataLen, bool eraseFlag)
{
	UINT32 *p = (UINT32*)dataBuff;
	UINT32 data;
	UINT32 writeAddr = chipAddr;
	/* Enable Prefetch Buffer */
	FLASH_PrefetchBufferCmd(ENABLE);
	/* Flash 2 wait state */
	FLASH_SetLatency(FLASH_Latency_2);
 	//FLASH_EraseOptionBytes();
	//FLASH_ProgramOptionByteData(0x1FFFF804, 0);
	//FLASH_EnableWriteProtection(FLASH_WRProt_AllPages);
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR | FLASH_FLAG_WRPERR); 
	if (eraseFlag) {
  	//(void)FLASH_ErasePage(chipAddr);
	}
	
	for(UINT32 i = 0; i < dataLen / sizeof(UINT32); i++)
	{
		data = *p++;
		(void)FLASH_ProgramWord(writeAddr , data);  
		writeAddr += sizeof(UINT32);
	}
	FLASH_Lock();	
	return SUCC;
}

/*
* @brief  chip flash page read, read 1 words(4 bytes) one operation
* @param  
* @retval 
*/
INT32 ChipFlashPagRead(UINT8 *dataBuff, UINT32 chipAddr, UINT32 dataLen)
{
	UINT32 *p = (UINT32*)dataBuff;
	UINT32 data;
	UINT32 readAddr = chipAddr;
	
	for(UINT32 i = 0; i < dataLen / sizeof(UINT32); i++)
	{
		data = *( UINT32*) readAddr;
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
UINT32 ChipFlashCheckSum(UINT32 pageAddress, UINT32 len)
{
	UINT32 checkSum=0;
	
 	for(UINT32 i = 0; i < len; i++) {
		checkSum += (*(UINT8*)(pageAddress + i));
	}

	return checkSum;
}

static struct sys_config g_sysConfig;

static void ReadSysConfigFromFlash(void)
{
	UINT8 *readBuff = (UINT8*)&g_sysConfig;
	ChipFlashPagRead(readBuff, SYS_CONFIG_ADDRESS, sizeof(g_sysConfig));
}

static void WriteSysConfigToFlash(void)
{
	UINT8 *writeBuff = (UINT8 *)&g_sysConfig;
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





