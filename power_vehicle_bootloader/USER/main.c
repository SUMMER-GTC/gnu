#include "stm32f4xx.h" 
#include "bsp_usart.h"
#include "bsp_timer.h"
#include "bsp_chip_flash.h"
#include "bsp_common_define.h"
#include "bsp_comm_protocol.h"
#include "core_cm4.h"
#include "bsp_led.h"

static const unsigned char g_bootDeviceNameBuf[] = "power vehicle";
static const unsigned char g_bootHardwareVersionBuf[] = "ver.1.0";

/*
* @brief  
* @param  
* @retval 
*/
s32 DeviceHardwareCheck(void)
{
	u32 t;
	u32 errByte=0;
	struct sys_config* sysConfig = GetSysConfigOpt()->sysConfig;
	for(t = 0; t < sizeof(g_bootDeviceNameBuf); t++) // check device name
	{
	  if(g_bootDeviceNameBuf[t] != sysConfig->deviceName[t]) {
			sysConfig->deviceNameErrCnt++;
			return FAIL;
		}
	}

	for(t = 0; t < sizeof(g_bootHardwareVersionBuf); t++) //check hardware version
	{
	  if(g_bootHardwareVersionBuf[t] != sysConfig->hardwareVer[t]) {
			sysConfig->hardWareErrCnt++;
			return FAIL;
		}
	}
	
	for(t = 0; t < ROM_SIZE_8K; t++) //check 8kbit rom 
	{
		if(((*(u8*)(APPLICATION_ADDRESS+t)) == 0xff) || ((*(u8*)(APPLICATION_ADDRESS+t)) == 0x00))
		{
		  errByte++;
			if(errByte >= 200) {
				sysConfig->romCheckErrCnt++;
				return FAIL;
			}
		}
		else
		{
			errByte=0;
		}
	}
	
	return SUCC;
}

/*
* @brief  
* @param  
* @retval 
*/
static void JumpToApplication(void)
{
	if(DeviceHardwareCheck() != SUCC) {
		GetSysConfigOpt()->sysConfig->otaState = OTA_RUN_BOOTLOADER;
		GetSysConfigOpt()->Write();
		//reset MCU	
		NVIC_SystemReset();
		return;
	}

	if (((*(u32*)APPLICATION_ADDRESS) & APPLICATION_SP_MASK ) == APPLICATION_SP_OK) {
		u32 jumpAddress = *(u32*) (APPLICATION_ADDRESS + 4);
		jump_t RunApplication = (jump_t) jumpAddress;

		/* Initialize user application's Stack Pointer */
		// __set_MSP(*(__IO u32*) APPLICATION_ADDRESS);
		RunApplication();
	}
    
  GetSysConfigOpt()->sysConfig->otaState = OTA_RUN_BOOTLOADER;
  GetSysConfigOpt()->Write();
  NVIC_SystemReset();
}

/*
* @brief  
* @param  
* @retval 
*/
static void BootloaderMain(void)
{
	while(1) {
		CommunicationProcess();
	}
}

static void GetSystemClockConfig(void)
{
  RCC_ClocksTypeDef RCC_Clocks = { 0 };
  RCC_GetClocksFreq(&RCC_Clocks);
  if (RCC_Clocks.SYSCLK_Frequency == SystemCoreClock) {
    return;
  }
  // printf frequency information
}

/*
* @brief  
* @param  
* @retval 
*/
int main(void)
{
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	__enable_irq();
	LedInit();
	TIM2_InitConfiguration();
	GetSystemClockConfig();
	delay_init();
	UsartInit();
	GetSysConfigOpt()->Init();

	if (GetSysConfigOpt()->sysConfig->otaState == OTA_RUN_APPLICATION) {
		JumpToApplication();
	}

	BootloaderMain();
}
/*********************************************END OF FILE**********************/
