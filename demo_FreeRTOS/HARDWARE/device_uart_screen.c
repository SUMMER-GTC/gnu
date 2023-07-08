#include "stdio.h"
#include "string.h"

#include "device_manager.h"
#include "device_uart_screen.h"
#include "init.h"

#include "sys.h"

#define UART_DGUS_MAX_LEN 64
#define UART_DGUS_MIN_LEN 2

static struct dgus_protocol g_uartScreenData;
static struct comm_dgus_data g_dgusData = {
	.state = COMM_IDLE_STATE,
	.rxCnt = 0,
	.dgusData = &g_uartScreenData
};

static struct code_update_protocol g_codeUpdate;
static struct comm_code_update_data g_codeUpdateData = {
	.state = COMM_IDLE_STATE,
	.rxCnt = 0,
	.codeUpdateData = &g_codeUpdate
};

static void DeviceUartInit(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
		 
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
		
		//USART2_TX 	GPIOA.2
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		 
		//USART2_RX 	GPIOA.3
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	
		//Usart1 NVIC 
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

static INT32 DeviceInit(void *dev)
{
	struct platform_info *pDev = (struct platform_info *)dev;
	pDev->states |= DEVICE_INIT;

	DeviceUartInit();

	PrintfLogInfo(DEBUG_LEVEL, "[device_uart_screen][DeviceInit]...\n");
	
	return SUCC;
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

static INT32 DeviceIoctl(void *dev, void *data, UINT32 dataLen)
{
	UNUSED(dev);
	if (dev == NULL) {

	}

	PrintfLogInfo(DEBUG_LEVEL, "[device_uart_screen][DeviceIoctl]...\n");

	return SUCC;
}

static INT32 DeviceProbe(void)
{
	PrintfLogInfo(DEBUG_LEVEL, "[device_uart_screen][DeviceProbe]...\n");

	return SUCC;
}

static struct file_operations g_fops = {
	.init = DeviceInit,
	.read = DeviceRead,
	.write = DeviceWrite,
	.ioctl = DeviceIoctl
};

static struct platform_info g_deviceDGUS = {
	.tag = TAG_DEVICE_UART_SCREEN,
	.fops = &g_fops,
	.private_data = (void *)&g_uartScreenData,
	.private_data_len = sizeof(g_uartScreenData),
	.setInterval = 0,
	.IntervalCall = NULL
};

static INT32 DeviceUartScreenInit(void)
{
	if (DeviceInit((void*)&g_deviceDGUS) != SUCC) {
		return FAIL;
	}

	if (DeviceProbe() != SUCC) {
		return FAIL;
	}

	RegisterDevice(&g_deviceDGUS);
	return SUCC;
}

module_init(DeviceUartScreenInit);

static UINT32 CheckSumCalculate(UINT8 *data, UINT32 dataLen)
{
	UINT32 checkSum = 0;

	for (UINT32 i = 0; i < dataLen; i++) {
		checkSum += data[i];
	}

	return checkSum;
}

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
				DeviceSampleData(SEND_FROM_ISR, TAG_APP_UI, &g_deviceDGUS);
			}
			break;
		default:
			comm->state = COMM_IDLE_STATE;
			break;
	}
}

static void CommCodeUpdateIrqHandler(UINT8 ch, struct comm_code_update_data *comm)
{
	UINT8 *rxBuff = (UINT8 *)comm->codeUpdateData;

	rxBuff[comm->rxCnt] = ch;

	switch(comm->state) {
		case COMM_IDLE_STATE:
			comm->rxCnt = 0;
			if (ch == COMM_CODE_UPDATE_SYN_HEAD1) {
				comm->state = COMM_SYN_HEAD1_STATE;
				comm->rxCnt++;
			}
			break;
		case COMM_SYN_HEAD1_STATE:
			if (ch == COMM_CODE_UPDATE_SYN_HEAD2) {
				comm->state = COMM_SYN_HEAD2_STATE;
				comm->rxCnt++;
			} else {
				comm->state = COMM_IDLE_STATE;
			}
			break;
		case COMM_SYN_HEAD2_STATE:
			if (comm->rxCnt++ >= (comm->codeUpdateData->dataLen + COMM_CODE_UPDATE_OFFSET(data))) {
				comm->state = COMM_IDLE_STATE;
				UINT32 checkSum = CheckSumCalculate(\
													(UINT8 *)&comm->codeUpdateData->mainCommand, \
													(comm->codeUpdateData->dataLen + COMM_CODE_UPDATE_OFFSET(data)));
				if (comm->codeUpdateData->checkSum == checkSum) {
					PrintfLogInfo(DEBUG_LEVEL, "code update!");
				}
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
		CommCodeUpdateIrqHandler(ch, &g_codeUpdateData);
 	}
}

