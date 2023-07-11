
#include "bsp_comm_protocol.h"
#include "bsp_common_define.h"
#include "bsp_usart.h"
#include "bsp_chip_flash.h"

static struct ota_protocol g_otaData;
static struct comm_ota_data g_commOtaData = {
	.receiveFinish = false,
	.state = COMM_IDLE_STATE,
	.rxCnt = 0,
	.otaData = &g_otaData,
};

struct comm_ota_data *GetCommunicationData(void)
{
	return &g_commOtaData;
}

extern struct sys_config_opt* GetSysConfigOpt(void);

//------------------------------
#define KEY_SBOX_ARRAY_SIZE 256

static const u8 g_keyData[] = {
        0x29, 0x23, 0xbe, 0x84, 0xe1, 0x6c, 0xd6, 0xae, 0x52, 0x90, 0x49, 0xf1, 0xf1, 0xbb, 0xe9, 0xeb,
        0xb3, 0xa6, 0xdb, 0x3c, 0x87, 0xc , 0x3e, 0x99, 0x24, 0x5e, 0xd , 0x1c, 0x6 , 0xb7, 0x47, 0xde,
//        0xb3, 0x12, 0x4d, 0xc8, 0x43, 0xbb, 0x8b, 0xa6, 0x1f, 0x3 , 0x5a, 0x7d, 0x9 , 0x38, 0x25, 0x1f,
//        0x5d, 0xd4, 0xcb, 0xfc, 0x96, 0xf5, 0x45, 0x3b, 0x13, 0xd , 0x89, 0xa , 0x1c, 0xdb, 0xae, 0x32,
//        0x20, 0x9a, 0x50, 0xee, 0x40, 0x78, 0x36, 0xfd, 0x12, 0x49, 0x32, 0xf6, 0x9e, 0x7d, 0x49, 0xdc,
//        0xad, 0x4f, 0x14, 0xf2, 0x44, 0x40, 0x66, 0xd0, 0x6b, 0xc4, 0x30, 0xb7, 0x32, 0x3b, 0xa1, 0x22,
//        0xf6, 0x22, 0x91, 0x9d, 0xe1, 0x8b, 0x1f, 0xda, 0xb0, 0xca, 0x99, 0x2 , 0xb9, 0x72, 0x9d, 0x49,
//        0x2c, 0x80, 0x7e, 0xc5, 0x99, 0xd5, 0xe9, 0x80, 0xb2, 0xea, 0xc9, 0xcc, 0x53, 0xbf, 0x67, 0xd6,
//        0xbf, 0x14, 0xd6, 0x7e, 0x2d, 0xdc, 0x8e, 0x66, 0x83, 0xef, 0x57, 0x49, 0x61, 0xff, 0x69, 0x8f,
//        0x61, 0xcd, 0xd1, 0x1e, 0x9d, 0x9c, 0x16, 0x72, 0x72, 0xe6, 0x1d, 0xf0, 0x84, 0x4f, 0x4a, 0x77,
//        0x2 , 0xd7, 0xe8, 0x39, 0x2c, 0x53, 0xcb, 0xc9, 0x12, 0x1e, 0x33, 0x74, 0x9e, 0xc , 0xf4, 0xd5,
//        0xd4, 0x9f, 0xd4, 0xa4, 0x59, 0x7e, 0x35, 0xcf, 0x32, 0x22, 0xf4, 0xcc, 0xcf, 0xd3, 0x90, 0x2d,
//        0x48, 0xd3, 0x8f, 0x75, 0xe6, 0xd9, 0x1d, 0x2a, 0xe5, 0xc0, 0xf7, 0x2b, 0x78, 0x81, 0x87, 0x44,
//        0xe , 0x5f, 0x50, 0x0 , 0xd4, 0x61, 0x8d, 0xbe, 0x7b, 0x5 , 0x15, 0x7 , 0x3b, 0x33, 0x82, 0x1f,
//        0x18, 0x70, 0x92, 0xda, 0x64, 0x54, 0xce, 0xb1, 0x85, 0x3e, 0x69, 0x15, 0xf8, 0x46, 0x6a, 0x4 ,
//        0x96, 0x73, 0xe , 0xd9, 0x16, 0x2f, 0x67, 0x68, 0xd4, 0xf7, 0x4a, 0x4a, 0xd0, 0x57, 0x68, 0x76,
};

static unsigned char g_keySbox[KEY_SBOX_ARRAY_SIZE];

/**
 * @brief  
 * @param  
 * @retval 
 */
void RC4Init(void)
{
    const unsigned char* data = g_keyData;
    unsigned int data_len = sizeof(g_keyData);

    unsigned int i;
    for (i = 0; i < KEY_SBOX_ARRAY_SIZE; ++i)
        g_keySbox[i] = i;

    unsigned int j = 0;
    // Initialize the S-box with the key
    for (i = 0; i < KEY_SBOX_ARRAY_SIZE; ++i) {
        unsigned char k = data[i % data_len];
        unsigned char swap_tmp;

        j = (g_keySbox[i] + j + k) % KEY_SBOX_ARRAY_SIZE;
        swap_tmp = g_keySbox[i];
        g_keySbox[i] = g_keySbox[j];
        g_keySbox[j] = swap_tmp;
    }
}

/**
 * @brief  
 * @param  
 * @retval 
 */
void RC4Crypt(unsigned char* data_in, unsigned int data_len)
{
    unsigned int i;
    unsigned int xorIndex;

    unsigned int x = 0;
    unsigned int y = 0;
    for (i = 0; i < data_len; ++i) {
        x = (x + 1) % KEY_SBOX_ARRAY_SIZE;
        y = (g_keySbox[x] + y) % KEY_SBOX_ARRAY_SIZE;

        unsigned char tmp = g_keySbox[x];
        g_keySbox[x] = g_keySbox[y];
        g_keySbox[y] = tmp;

        xorIndex = (g_keySbox[x] + g_keySbox[y]) % KEY_SBOX_ARRAY_SIZE;
        data_in[i] = data_in[i] ^ g_keySbox[xorIndex];
    }
}

/**
 * @brief  
 * @param  
 * @retval 
 */
void DataEncrypt(unsigned char *data, unsigned int dataLen)
{
    RC4Init();
    RC4Crypt(data, dataLen);
}

/**
 * @brief  
 * @param  
 * @retval 
 */
void DataDecrypt(unsigned char *data, unsigned int dataLen)
{
    RC4Init();
    RC4Crypt(data, dataLen);
}

static u32 CheckSumCalculate(u8 *data, u32 dataLen)
{
	u32 checkSum = 0;

	for (u32 i = 0; i < dataLen; i++) {
		checkSum += data[i];
	}

	return checkSum;
}

/*
* @brief  
* @param  
* @retval 
*/
void CommOtaIrqHandler(u8 ch)
{
	struct comm_ota_data *comm = &g_commOtaData;
	u8 *rxBuff = (u8 *)comm->otaData;

	rxBuff[comm->rxCnt] = ch;

	switch(comm->state) {
		case COMM_IDLE_STATE:
			comm->rxCnt = 0;
			if (ch == COMM_OTA_SYN_HEAD1) {
				comm->state = COMM_SYN_HEAD1_STATE;
				++comm->rxCnt;
			}
			break;
		case COMM_SYN_HEAD1_STATE:
			if (ch == COMM_OTA_SYN_HEAD2) {
				comm->state = COMM_SYN_HEAD2_STATE;
				++comm->rxCnt;
			} else {
				comm->state = COMM_IDLE_STATE;
			}
			break;
		case COMM_SYN_HEAD2_STATE:
			if (++comm->rxCnt >= (comm->otaData->dataLen + COMM_OTA_OFFSET(data))) {
				comm->state = COMM_IDLE_STATE;
				u32 checkSum = CheckSumCalculate(\
													(u8 *)&comm->otaData->mainCommand, \
													(comm->otaData->dataLen + COMM_OTA_OFFSET(data) - COMM_OTA_OFFSET(mainCommand)));
				if (comm->otaData->checkSum == checkSum) {
					comm->receiveFinish = true;
				}
			}
			break;
		default:
			comm->state = COMM_IDLE_STATE;
			break;
	}
}


#define UASRT_SEND_BUFF_SIZE (32)
static bool OtaReplyProcess(struct ota_protocol *comData)
{
 	u8 *sendBuff = NULL;
	sendBuff = (u8 *)malloc(UASRT_SEND_BUFF_SIZE);
	if (sendBuff == NULL) {
		return false;
	}

	u8 *dataPtr = (u8 *)comData;
	u16 dataLen = COMM_OTA_OFFSET(data);

 	if (dataLen > UASRT_SEND_BUFF_SIZE) {
		dataLen = UASRT_SEND_BUFF_SIZE;
	}
	
	for (u16 i = 0; i < dataLen; i++) {
		sendBuff[i] = dataPtr[i];
	}

	UsartSend(sendBuff, dataLen);

	free(sendBuff);
  return true;
}

u32 g_programAddr = APPLICATION_ADDRESS;
static s32 OtaSynHead(struct ota_protocol *otaData)
{
	u32 startAddr = *(u32*)otaData->data;

	if (startAddr < APPLICATION_ADDRESS) {
		return FAIL;
	}

	if (startAddr == APPLICATION_ADDRESS) {
		ChipFlashEraseAppRom();
	}

	g_programAddr = startAddr;
	OtaReplyProcess(otaData);
	return SUCC;
}

static u8 g_lastDataBuff[32] = { 0 };
static u32 g_flashCheckSum = 0;
static s32 OtaProgramFlash(struct ota_protocol *otaData)
{
	u8* dataBuff = otaData->data;
	u32 dataLen = otaData->dataLen;

	if (dataLen >= SYS_CONFIG_ADDRESS) {
		return FAIL;
	}

	ChipFlashPageWrite(dataBuff, g_programAddr, dataLen, false);

	u32 i;
	for (i = 0; i < sizeof(g_lastDataBuff); i++) {
		if (g_lastDataBuff[i] != dataBuff[i]) {
			break;
		}
	}

	if (i != sizeof(g_lastDataBuff)) {
		g_flashCheckSum += ChipFlashCheckSum(g_programAddr, dataLen);		
		g_programAddr += dataLen;
		memcpy(g_lastDataBuff, dataBuff, sizeof(g_lastDataBuff));
	}
	
	OtaReplyProcess(otaData);

	return SUCC;
}

static const unsigned char g_bootDeviceNameBuf[] = "this device is test!";
static const unsigned char g_bootHardwareVersionBuf[] = "VER.1.0"; 

/*
* @brief  
* @param  
* @retval 
*/
static s32 DeviceHardwareCheck(void)
{
	u32 t;
	u32 errByte=0;
	struct sys_config* sysConfig = GetSysConfigOpt()->sysConfig;
	for(t = 0; t < sizeof(g_bootDeviceNameBuf); t++) // check device name
	{
	  if(g_bootDeviceNameBuf[t] != (*(u8*)(DEVICE_NAME_BUFF_ADDRESS+t))) {
			sysConfig->deviceNameErrCnt++;
			return FAIL;
		}
	}

	for(t = 0; t < sizeof(g_bootHardwareVersionBuf); t++) //check hardware version
	{
	  if(g_bootHardwareVersionBuf[t] != (*(u8*)(HARDWARE_VERSION_BUFF_ADDRESS+t))) {
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

static void OtaGoApplication(struct ota_protocol *otaData)
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

    OtaReplyProcess(otaData);
		/* Initialize user application's Stack Pointer */
		__set_MSP(*(__IO u32*) APPLICATION_ADDRESS);
		RunApplication();
	}
  
  NVIC_SystemReset();
}

static s32 OtaRunApplication(struct ota_protocol *otaData)
{
	GetSysConfigOpt()->sysConfig->otaState = OTA_RUN_APPLICATION;
	GetSysConfigOpt()->Write();
	OtaGoApplication(otaData);
	return SUCC;
}

static s32 OtaCheckSumFlash(struct ota_protocol *otaData)
{
	u32 otaCheckSum = *(u32*)otaData->data;
	u32 flashCheckSum = g_flashCheckSum;
	g_flashCheckSum = 0;

	if (flashCheckSum != otaCheckSum) {
		return FAIL;
	}

	OtaRunApplication(otaData);

	return SUCC;
}

static s32 OtaRunBootloader(struct ota_protocol *otaData)
{
	OtaReplyProcess(otaData);
	GetSysConfigOpt()->sysConfig->otaState = OTA_RUN_BOOTLOADER;
	GetSysConfigOpt()->Write();
	NVIC_SystemReset();
	return SUCC;
}

static s32 OtaProcess(struct ota_protocol *otaData)
{
	struct ota_protocol *pOta = otaData;
	
	if (pOta->subCommand >= OTA_SUB_COMMAND_END) {
		return FAIL;
	}

	switch(pOta->subCommand) {
		case OTA_SYN_HEAD:
			OtaSynHead(otaData);
			break;
		case OTA_PROGRAM_FLASH:
			OtaProgramFlash(otaData);
			break;
		case OTA_CHECKSUM_FLASH:
			OtaCheckSumFlash(otaData);
			break;
		case OTA_RUN_BOOTLOADER:
			OtaRunBootloader(otaData);
			break;
		case OTA_RUN_APPLICATION:
			OtaRunApplication(otaData);
			break;
		default:
			break;
	}

	return SUCC;
}

static struct main_command_process g_mainCommandProcess[MAIN_COMMAND_END] = {
	{MAIN_COMMAND_OTA, OtaProcess},
};

void CommunicationProcess(void)
{
	if (g_commOtaData.receiveFinish == true) {
    g_commOtaData.receiveFinish = false;
		for (u8 i = 0; i < MAIN_COMMAND_END; i++) {
			if (g_mainCommandProcess[i].mainCommand == g_commOtaData.otaData->mainCommand) {
				g_mainCommandProcess[i].mainCommandProcess(g_commOtaData.otaData);
			} else {
				// printf error command
			}
		}
	}
}


// end of bsp_com_protocol.c
