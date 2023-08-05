#include "device_manager.h"
#include "usart_screen.h"
#include "init.h"

#include "sys.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"

#define UART_DGUS_MAX_LEN (64)
#define UART_DGUS_MIN_LEN (2)
#define UART_SCREEN_USE_DMA_SEND (1)

enum {
	COMM_IDLE_STATE = 0,
	COMM_SYN_HEAD1_STATE = 1,
	COMM_SYN_HEAD2_STATE,
	COMM_DATA_LEN_STATE
};

static struct dgus_protocol g_uartScreenData;
static struct comm_dgus_data g_dgusData = {
	.state = COMM_IDLE_STATE,
	.rxCnt = 0,
	.dgusData = &g_uartScreenData
};

static UINT8 g_uartScreenDmaSendBuff[64] = { 0 };

static void DeviceUartInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
		
	RCC_APB2PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
		
	//USART2_TX 	GPIOA.2
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
		
	//USART2_RX 	GPIOA.3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	//USART NVIC 
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=4 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	//USART init set
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART2, &USART_InitStructure);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART2, ENABLE);		
}

static void UartScreenDMAConfig(void)
{
#if UART_SCREEN_USE_DMA_SEND
  DMA_InitTypeDef DMA_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

  DMA_DeInit(DMA1_Stream6);
  while (DMA_GetCmdStatus(DMA1_Stream6) != DISABLE) {
  }

  DMA_InitStructure.DMA_Channel = DMA_Channel_4;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = ((UINT32)&USART2->DR);
  DMA_InitStructure.DMA_Memory0BaseAddr = (UINT32)g_uartScreenDmaSendBuff;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;	
  DMA_InitStructure.DMA_BufferSize = sizeof(g_uartScreenDmaSendBuff);
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
  DMA_Init(DMA1_Stream6, &DMA_InitStructure);
  
	USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
#else
	return;
#endif
}

static INT32 DeviceInit(void)
{
	DeviceUartInit();
	UartScreenDMAConfig();

	PrintfLogInfo(DEBUG_LEVEL, "[device_uart_screen][DeviceInit]...\n");
	
	return SUCC;
}

static void DgusUartSend(UINT8 *data, UINT16 dataLen)
{
#if UART_SCREEN_USE_DMA_SEND
	if (dataLen > sizeof(g_uartScreenDmaSendBuff)) {
		dataLen = sizeof(g_uartScreenDmaSendBuff);
	}
	memcpy(g_uartScreenDmaSendBuff, data, dataLen);
	DMA_Cmd(DMA1_Stream6, DISABLE);
	DMA_SetCurrDataCounter(DMA1_Stream6, dataLen);
  DMA_Cmd(DMA1_Stream6, ENABLE);
	while(DMA_GetFlagStatus(DMA1_Stream6, DMA_FLAG_TCIF6) == RESET) {
	}
	DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_TCIF6);
#else
	UINT8 *pData = data;
	
	for (UINT16 i = 0; i < dataLen; i++) {
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);    
		USART_SendData(USART2, pData[i]);
	}
#endif
}

static void DgusWriteData( UINT16 wAddress, INT16 wData )
{
	UINT8 aucDgusCmd[8];

	aucDgusCmd[0] = DGUS_FRAME_HEADER0;
	aucDgusCmd[1] = DGUS_FRAME_HEADER1;

	aucDgusCmd[2] = 0x05;

	aucDgusCmd[3] = DGUS_CMD_MEM_WR;

	aucDgusCmd[4] = (wAddress >> 8);
	aucDgusCmd[5] = wAddress;


	aucDgusCmd[6] = GET_UINT16_H(wData);
	aucDgusCmd[7] = GET_UINT16_L(wData);

	DgusUartSend(aucDgusCmd, 8);
}

static void DgusWriteString( UINT16 wAddress, const UINT8 *pDatBuf, UINT8 ucDecLen )
{
	UINT8 ucLoop = 0;
	UINT8 aucDgusCmd[32];

	aucDgusCmd[0] = DGUS_FRAME_HEADER0;
	aucDgusCmd[1] = DGUS_FRAME_HEADER1;

	aucDgusCmd[2] = ucDecLen + 3;

	aucDgusCmd[3] = DGUS_CMD_MEM_WR;


	aucDgusCmd[4] = GET_UINT16_H( wAddress );
	aucDgusCmd[5] = GET_UINT16_L( wAddress );

	for( ucLoop = 0; ucLoop < ucDecLen; ucLoop++ )
	{
		aucDgusCmd[6 + ucLoop] = pDatBuf[ucLoop];
	}
	
	DgusUartSend(aucDgusCmd, ucDecLen + 6);
}

static void DgusSelectPage( UINT16 wPicId )
{
	UINT8 aucDgusCmd[7];

	aucDgusCmd[0] = DGUS_FRAME_HEADER0;
	aucDgusCmd[1] = DGUS_FRAME_HEADER1;

	aucDgusCmd[2] = 0x04;

	aucDgusCmd[3] = DGUS_CMD_REG_WR;

	aucDgusCmd[4] = 0x03;

	aucDgusCmd[5] = GET_UINT16_H(wPicId);
	aucDgusCmd[6] = GET_UINT16_L(wPicId);

	DgusUartSend(aucDgusCmd, 7);
}

static INT32 DeviceRead(void *dev)
{
	UNUSED(dev);

	PrintfLogInfo(DEBUG_LEVEL, "[device_uart_screen][DeviceRead]\n");

	return SUCC;
}

static INT32 DeviceWrite(void *dev, void *data, UINT32 dataLen)
{
	UNUSED(dev);

	PrintfLogInfo(DEBUG_LEVEL, "[device_uart_screen][DeviceWrite]...\n");

	return SUCC;
}

static void DgusDataWriteCmd(void *data, UINT32 dataLen)
{
	struct dgus_addr_data *addrData = (struct dgus_addr_data *)data;
	DgusWriteData(addrData->addr, *(UINT16 *)addrData->data);
}

static void DgusTextWriteCmd(void *data, UINT32 dataLen)
{
	struct dgus_addr_data *addrData = (struct dgus_addr_data *)data;
	DgusWriteString(addrData->addr, addrData->data, dataLen - sizeof(addrData->addr));
}

static void DgusPageWriteCmd(void *data, UINT32 dataLen)
{
	if (dataLen > sizeof(UINT16)) {
		return;
	}
	DgusSelectPage(*(UINT16 *)data);
}

static INT32 DeviceIoctl(void *dev, UINT32 cmd, void *data, UINT32 dataLen)
{
	switch(cmd) {
		case DGUS_DATA_W_CMD:
			DgusDataWriteCmd(data, dataLen);
			break;
		case DGUS_DATA_R_CMD:
			break;
		case DGUS_TEXT_W_CMD:
			DgusTextWriteCmd(data, dataLen);
			break;
		case DGUS_TEXT_R_CMD:
			break;
		case DGUS_PAGE_W_CMD:
			DgusPageWriteCmd(data, dataLen);
			break;
		case DGUS_PAGE_R_CMD:
			break;
		default: break;
	}

	return SUCC;
}

UINT8 g_dgusCheckPowerOnFlag = 0;
static INT32 DgusCheckPowerOn( void )
{
	UINT8 aucDgusCmd[6];
	
	aucDgusCmd[0] = DGUS_FRAME_HEADER0;
	aucDgusCmd[1] = DGUS_FRAME_HEADER1;
	aucDgusCmd[2] = 0x03;
	aucDgusCmd[3] = DGUS_CMD_REG_RD;
	aucDgusCmd[4] = 0x03;
	aucDgusCmd[5] = 0x02;

	for(UINT8 ucLoop = 0; ucLoop < 3; ucLoop++) {
		DgusUartSend( aucDgusCmd, 6 );
		delay_ms(50);
		if(g_dgusCheckPowerOnFlag) {
			return SUCC;
		}
	}

	return FAIL;
}

static INT32 DeviceProbe(void)
{
	if (DgusCheckPowerOn() != SUCC) {
		return FAIL;
	}
	DgusSelectPage(PAGE_HOME);
	PrintfLogInfo(DEBUG_LEVEL, "[device_uart_screen][DeviceProbe]...\n");
	return SUCC;
}

static struct file_operations g_fops = {
	.read = DeviceRead,
	.write = DeviceWrite,
	.ioctl = DeviceIoctl
};

static struct platform_info g_deviceDGUS = {
	.tag = TAG_DEVICE_UART_SCREEN,
	.fops = &g_fops,
	.private_data = (void *)&g_uartScreenData,
	.private_data_len = sizeof(g_uartScreenData),
	.states = 0,
};

static INT32 DeviceUartScreenInit(void)
{
	if (DeviceInit() != SUCC) {
		return FAIL;
	}

	if (DeviceProbe() != SUCC) {
		return FAIL;
	}

	g_deviceDGUS.states |= DEVICE_INIT;
	RegisterDevice(&g_deviceDGUS);
	return SUCC;
}

module_init(DeviceUartScreenInit);

static void CommDgusIrqHandler(UINT8 ch, struct comm_dgus_data *comm)
{
	UINT8 *rxBuff = (UINT8 *)comm->dgusData;

	rxBuff[comm->rxCnt] = ch;

	switch(comm->state) {
		case COMM_IDLE_STATE:
			comm->rxCnt = 0;
			if (ch == COMM_DGUS_SYN_HEAD1) {
				comm->state = COMM_SYN_HEAD1_STATE;
				comm->rxCnt++;
			}
			break;
		case COMM_SYN_HEAD1_STATE:
			if (ch == COMM_DGUS_SYN_HEAD2) {
				comm->state = COMM_SYN_HEAD2_STATE;
				comm->rxCnt++;
			} else {
				comm->state = COMM_IDLE_STATE;
			}
			break;
		case COMM_SYN_HEAD2_STATE:
			if (ch >= UART_DGUS_MIN_LEN && ch <= UART_DGUS_MAX_LEN) {
				comm->state = COMM_DATA_LEN_STATE;
				comm->rxCnt++;
			} else {
				comm->state = COMM_IDLE_STATE;
			}
			break;
		case COMM_DATA_LEN_STATE:
			if (comm->rxCnt++ >= (comm->dgusData->dataLen + COMM_DGUS_OFFSET(command) - 1)) {
				comm->state = COMM_IDLE_STATE;
				g_dgusCheckPowerOnFlag = 1;
				DeviceSampleData(SEND_FROM_ISR, TAG_APP_UI, &g_deviceDGUS);
			}
			break;
		default:
			comm->state = COMM_IDLE_STATE;
			break;
	}
}

void USART2_IRQHandler(void)
{
	UINT8 ch;

 	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
		ch = USART_ReceiveData(USART2);
		CommDgusIrqHandler(ch, &g_dgusData);
 	}
}

