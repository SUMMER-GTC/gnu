/**
  ******************************************************************************
  * @file    usart_upper_computer.c
  * @author  
  * @version 
  * @date    
  * @brief   
  ******************************************************************************
  */
  
#include "usart_upper_computer.h"
#include "device_manager.h"
#include "usart_screen.h"
#include "init.h"

#include "sys.h"

#define UPPER_COMPUTER_SEND_BUFF_SIZE (256)
#define UART_UPPER_COMPUTER_DATA_SIZE (10)

#define USART_UPPER_COMPUTER_USE_DMA_SEND (1)

enum {
	COMM_IDLE_STATE = 0,
	COMM_SYN_HEAD1_STATE = 1,
	COMM_SYN_HEAD2_STATE,
	COMM_DATA_LEN_STATE
};

static UINT8 g_upperComputerSendBuff[UPPER_COMPUTER_SEND_BUFF_SIZE] = { 0 };
static UINT8 g_rd800[UART_UPPER_COMPUTER_DATA_SIZE] = { 0 };

static struct ota_protocol g_codeUpdate = { 
	.synHead = COMM_OTA_SYN_HEAD,
	.checkSum = 0,
	.mainCommand = 0,
	.subCommand = 0,
	.dataLen = 0,
	.data = { 0 }
};

static struct comm_ota_data g_codeUpdateData = {
	.state = COMM_IDLE_STATE,
	.rxCnt = 0,
	.otaData = &g_codeUpdate
};

static struct upper_computer g_upperComputer = {
	.dataType = UPPER_COMPUTER_RD800_DATA,
	.ota = &g_codeUpdate,
	.rd800 = g_rd800
};

static void DeviceUartInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
	
	/* USART1 mode config */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure; 
	/* Enable the USARTy Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

static void UartUpperComputerDMAConfig(void)
{
#if USART_UPPER_COMPUTER_USE_DMA_SEND
  DMA_InitTypeDef DMA_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

  DMA_DeInit(DMA2_Stream7);
  while (DMA_GetCmdStatus(DMA2_Stream7) != DISABLE) {
  }

  DMA_InitStructure.DMA_Channel = DMA_Channel_4;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = ((UINT32)&USART1->DR);
  DMA_InitStructure.DMA_Memory0BaseAddr = (UINT32)g_upperComputerSendBuff;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;	
  DMA_InitStructure.DMA_BufferSize = sizeof(g_upperComputerSendBuff);
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;

  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;      
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;        
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;    
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;    
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;      
  DMA_Init(DMA2_Stream7, &DMA_InitStructure);
  
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
#else
	return;
#endif
}

static void UsarUpperComputertSend(UINT8 *data, UINT16 dataLen)
{
#if USART_UPPER_COMPUTER_USE_DMA_SEND
	if (dataLen > sizeof(g_upperComputerSendBuff)) {
		dataLen = sizeof(g_upperComputerSendBuff);
	}
	memcpy(g_upperComputerSendBuff, data, dataLen);
	DMA_SetCurrDataCounter(DMA2_Stream7, dataLen);
  DMA_Cmd(DMA2_Stream7, ENABLE);
	while(DMA_GetFlagStatus(DMA2_Stream7, DMA_FLAG_TCIF7) == RESET) {
	}
	DMA_ClearFlag(DMA2_Stream7, DMA_FLAG_TCIF7);
#else
	UINT8 *pData = data;
	
	for (UINT16 i = 0; i < dataLen; i++) {
		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);    
		USART_SendData(USART1, pData[i]);
	}
#endif
}

static INT32 DeviceInit(void *dev)
{
	struct platform_info *pDev = (struct platform_info *)dev;
	pDev->states |= DEVICE_INIT;

	DeviceUartInit();
	UartUpperComputerDMAConfig();

	PrintfLogInfo(DEBUG_LEVEL, "[usart_upper_computer][DeviceInit]...\n");
	
	return SUCC;
}

static INT32 DeviceRead(void *dev)
{
	UNUSED(dev);


	return SUCC;
}

static INT32 DeviceWrite(void *dev, void *data, UINT32 dataLen)
{
	UNUSED(dev);

	UsarUpperComputertSend((UINT8 *)data, dataLen);
	return SUCC;
}

static INT32 DeviceProbe(void)
{
	PrintfLogInfo(DEBUG_LEVEL, "[usart_upper_computer][DeviceProbe]...\n");

	return SUCC;
}

static struct file_operations g_fops = {
	.read = DeviceRead,
	.write = DeviceWrite,
};

static struct platform_info g_deviceUpperComputer = {
	.tag = TAG_DEVICE_UPPER_COMPUTER,
	.fops = &g_fops,
	.private_data = (void *)&g_upperComputer,
	.private_data_len = sizeof(g_upperComputer),
};

static INT32 DeviceUpperComputerInit(void)
{
	if (DeviceInit((void*)&g_deviceUpperComputer) != SUCC) {
		return FAIL;
	}

	if (DeviceProbe() != SUCC) {
		return FAIL;
	}

	RegisterDevice(&g_deviceUpperComputer);
	return SUCC;
}

module_init(DeviceUpperComputerInit);

static UINT32 CheckSumCalculate(UINT8 *data, UINT32 dataLen)
{
	UINT32 checkSum = 0;

	for (UINT32 i = 0; i < dataLen; i++) {
		checkSum += data[i];
	}

	return checkSum;
}

static UINT16 g_receiveCnt = 0;
static void CommOtaIrqHandler(UINT8 ch, struct comm_ota_data *comm)
{
	UINT8 *rxBuff = (UINT8 *)comm->otaData;

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
				UINT32 checkSum = CheckSumCalculate(\
													(UINT8 *)&comm->otaData->mainCommand, \
													(comm->otaData->dataLen + COMM_OTA_OFFSET(data) - COMM_OTA_OFFSET(mainCommand)));
				if (comm->otaData->checkSum == checkSum) {
					++g_receiveCnt;
					g_upperComputer.dataType = UPPER_COMPUTER_OTA_DATA;
					DeviceSampleData(SEND_FROM_ISR, TAG_APP_COMPUTER, &g_deviceUpperComputer);
				}
			}
			break;
		default:
			comm->state = COMM_IDLE_STATE;
			break;
	}
}

void USART1_IRQHandler(void)
{
	UINT8 ch;

 	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
		ch = USART_ReceiveData(USART1);
		CommOtaIrqHandler(ch, &g_codeUpdateData);
 	}
}

/*********************************************END OF FILE**********************/
