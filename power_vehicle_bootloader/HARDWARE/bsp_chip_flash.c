#include "stm32f4xx_flash.h"
#include "bsp_chip_flash.h"
#include "bsp_common_define.h"
#include "bsp_comm_protocol.h"

#define CHIP_SECTOR_TOTAL_NUMBER (12)
#define CHIP_APPLICATION_START_SECTOR_NUMBER (2)
#define CHIP_FLASH_SIZE_1KB (1024)

struct flash_sector g_flashSector[CHIP_SECTOR_TOTAL_NUMBER] = {
	{16, FLASH_Sector_0}, 		// 16KB
	{32, FLASH_Sector_1}, 		// 16KB
	{48, FLASH_Sector_2}, 		// 16KB
	{64, FLASH_Sector_3}, 		// 16KB
	{128, FLASH_Sector_4}, 		// 64KB
	{256, FLASH_Sector_5}, 		// 128KB
	{384, FLASH_Sector_6}, 		// 128KB
	{512, FLASH_Sector_7}, 		// 128KB
	{640, FLASH_Sector_8}, 		// 128KB
	{768, FLASH_Sector_9}, 		// 128KB
	{896, FLASH_Sector_10}, 	// 128KB
	{1024, FLASH_Sector_11}, 	// 128KB
};

void ChipFlashEraseAppRom(u32 codeEndAddr)
{
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGAERR | FLASH_FLAG_WRPERR);

	if (codeEndAddr  > (BOOTLOADER_ADDRESS + g_flashSector[CHIP_SECTOR_TOTAL_NUMBER - 1].sectorEndAddr * CHIP_FLASH_SIZE_1KB)) {
		return;
	}

	u16 endSector = APPLICATION_END_SECTOR;
	for (u8 i = CHIP_APPLICATION_START_SECTOR_NUMBER; i < CHIP_SECTOR_TOTAL_NUMBER; i++) {
		if (BOOTLOADER_ADDRESS + g_flashSector[i].sectorEndAddr * CHIP_FLASH_SIZE_1KB >= codeEndAddr) {
			endSector = g_flashSector[i].sectorNum;
			break;
		}
	}

	for (u16 sector = APPLICATION_START_SECTOR; sector <= endSector; sector += APPLICATION_SECTOR_OFFSET) {
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
	ReadSysConfigFromFlash();
	if (g_sysConfig.firstUseId != FIRST_USE_ID) {
		g_sysConfig.firstUseId = FIRST_USE_ID;
		g_sysConfig.hardWareErrCnt = 0;
		g_sysConfig.deviceNameErrCnt = 0;
		g_sysConfig.romCheckErrCnt = 0;
		g_sysConfig.otaState = OTA_RUN_APPLICATION;
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





