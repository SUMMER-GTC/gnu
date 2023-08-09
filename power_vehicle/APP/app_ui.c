#include "app_ui.h"
#include "app_computer.h"

#include "app_manager.h"
#include "app_task_define.h"
#include "init.h"
#include "stdio.h"
#include "queue.h"
#include "device_manager.h"	
#include "timers.h"

#define RMP_WARNING_VALUE 400

static UINT16 g_dgusPage = 0;
static bool g_startRun = false;

static INT32 UiSendData(UINT8 desTag, void *data, UINT16 dataLen)
{
	return SendDataToQueue(TAG_APP_UI, desTag, data, dataLen);
}

static void UiWriteData(struct platform_info *dev, UINT16 addr, UINT16 value)
{
	struct dgus_addr_data dgus;

	dgus.addr = addr;
	dgus.data[0] = value;
	dev->fops->ioctl(dev, DGUS_DATA_W_CMD, (void*)&dgus, sizeof(dgus.addr) + sizeof(UINT16));
}

static void UiUpdate(struct platform_info *dev, struct ui_display_data *uiData, UINT16 flag)
{
	static UINT16 lastRpm = 0;

	UiWriteData(dev, DATA_RPM, uiData->rpm);
	// change font color
	if (uiData->rpm <= RMP_WARNING_VALUE && lastRpm > RMP_WARNING_VALUE) {
		UiWriteData(dev, DATA_RPM_SP_COLOR, DATA_RPM_RED);
		UiWriteData(dev, DATA_RPM_SP_COLOR, DATA_RPM_RED);
	} else if (uiData->rpm > RMP_WARNING_VALUE && lastRpm <= RMP_WARNING_VALUE) {
		UiWriteData(dev, DATA_RPM_SP_COLOR, DATA_RPM_GREEN);
		UiWriteData(dev, DATA_RPM_SP_COLOR, DATA_RPM_GREEN);
	}

	lastRpm = uiData->rpm;

	if (flag == PAGE_HEART_LUNG) {
		UiWriteData(dev, DATA_SPO2, uiData->spo2);
		UiWriteData(dev, DATA_VO2, uiData->vo2);
		UiWriteData(dev, DATA_VCO2, uiData->vco2);
		UiWriteData(dev, DATA_HR, uiData->heartRate);
		UiWriteData(dev, DATA_LBP, uiData->lbp);
		UiWriteData(dev, DATA_HBP, uiData->hbp);		
	}

}

static void UiAppProcessUi(struct platform_info *app)
{
	struct ui_display_data *uiData = (struct ui_display_data *)app->private_data;
	if (!g_startRun) {
		return;
	}

	struct platform_info *dev;
	static UINT16 lastSpo2 = 0xFFFF;
	if (GetDeviceInfo(TAG_DEVICE_UART_SCREEN, &dev) == FAIL) {
		return;
	}

	if (uiData->spo2 != 0xFFFF && lastSpo2 == 0xFFFF) {
		g_dgusPage = PAGE_HEART_LUNG;
		dev->fops->ioctl(dev, DGUS_PAGE_W_CMD, (void*)&g_dgusPage, sizeof(g_dgusPage));
	} else if (uiData->spo2 == 0xFFFF && lastSpo2 != 0xFFFF) {
		g_dgusPage = PAGE_STANDALONE;
		dev->fops->ioctl(dev, DGUS_PAGE_W_CMD, (void*)&g_dgusPage, sizeof(g_dgusPage));
	}

	lastSpo2 = uiData->spo2;
	UiUpdate(dev, uiData, g_dgusPage);

}

static void UiAppProcess(struct platform_info *data)
{
	switch(data->tag) {
		case TAG_APP_COMPUTER:
			UiAppProcessUi(data);
			break;
		default:
		
			break;
	}
}

static bool DgusPackageAnalyze( UINT16 *pAddress, UINT16 *pKeyData, UINT8 *aucRxBuf )
{
	if((pAddress == NULL) || (pKeyData == NULL)) {
		return false;
	}

  UINT16 data=0;
	UINT16 address=0;

	if( DGUS_CMD_MEM_RD == aucRxBuf[DGUS_MEMORD_CMD] ) {
		address |= aucRxBuf[DGUS_MEMORD_ADDH];
		address <<= 8;
		address |= aucRxBuf[DGUS_MEMORD_ADDL];
		
		data |= aucRxBuf[DGUS_MEMORD_DATAH];
		data <<= 8;
		data |= aucRxBuf[DGUS_MEMORD_DATAL];
	}

	*pAddress = address;
	*pKeyData = data;

	return true;
}

static void PowerIncDecProcess(struct platform_info *dev, UINT16 value)
{
	UiWriteData(dev, DATA_POWER, value);
}

static UINT16 g_timerSecond = 0;
static void StartProcess(struct platform_info *dev)
{
	g_startRun = !g_startRun;

	if (g_startRun) {
		g_timerSecond = 0;
		UiWriteData(dev, DATA_TIME, g_timerSecond);
	}

	dev->fops->ioctl(dev, UART_SCREEN_START_TIMER_CMD, (void*)&g_startRun, sizeof(g_startRun));
}

static void UiLogDelayProcess(struct platform_info *dev)
{
	// change to home page 
	g_dgusPage = PAGE_STANDALONE;
	dev->fops->ioctl(dev, DGUS_PAGE_W_CMD, (void*)&g_dgusPage, sizeof(g_dgusPage));
}

static void UiTimerProcess(struct platform_info *dev)
{
	++g_timerSecond;
	UiWriteData(dev, DATA_TIME, g_timerSecond);
}

static void UiDgusProcess(struct platform_info *dev, UINT8 *data)
{
	UINT16 keyAddr = 0;
	UINT16 keyData = 0;
	if (!DgusPackageAnalyze(&keyAddr, &keyData, data)) {
		return;
	}

	switch( keyAddr ) {
		case KEY_RETURN_POWER_INC_DEC:
			PowerIncDecProcess(dev, keyData);
			break;
		case KEY_RETURN_START:
			StartProcess(dev);
			break;
		default:
			break;
	}
}

static void UiDeviceProcessUartScreen(struct platform_info *dev)
{
	struct uart_screen* pUartScreen = (struct uart_screen *)dev->private_data;
	UINT8 dataType = pUartScreen->dataType;
	UINT8 *pData = pUartScreen->data;

	switch(dataType) {
		case UART_SCREEN_LOG_DELAY_DATA:
			UiLogDelayProcess(dev);
			break;
		case UART_SCREEN_TIMER_DATA:
			UiTimerProcess(dev);
			break;
		case UART_SCREEN_DGUS_DATA:
			UiDgusProcess(dev, pData);
			break;
		default:
			break;
	}
	PrintfLogInfo(DEBUG_LEVEL, "[app_ui][UiDeviceProcessUartScreen]\n");
}

static void UiDeviceProcess(struct platform_info *data)
{
	struct platform_info *pData = data;
	switch(pData->tag) {
		case TAG_DEVICE_UART_SCREEN:
			UiDeviceProcessUartScreen(pData);
			break;
		default:
		
			break;
	}
}

void UiTask(void *pvParameters)
{
	struct platform_info queueData;
	QueueHandle_t xQueue = (QueueHandle_t)pvParameters;
	for(;;) {
		xQueueReceive(xQueue, &queueData, portMAX_DELAY);
		
		UINT8 cnt = uxQueueSpacesAvailable(xQueue);
		PrintfLogInfo(DEBUG_LEVEL, "[app_ui][UiTask] queue remain %d\n", cnt);

		if (queueData.tag < TAG_APP_END) {
			UiAppProcess(&queueData);
		} else if (queueData.tag >= TAG_DEVICE_START && queueData.tag < TAG_DEVICE_END) {
			UiDeviceProcess(&queueData);
		} else {
			// unknow tag error
		}
	}

}

static UINT8 g_uiData[COMPUTER_DATA_BUFF_SIZE] = { 0 };

static struct platform_info g_appUi = {
	.tag = TAG_APP_UI,
	.private_data = g_uiData,
};

static INT32 AppUiInit(void)
{
	static TaskHandle_t pUiTCB = NULL;
	TaskInit(g_appUi.tag, UiTask, &pUiTCB);
	RegisterApp(&g_appUi);
	return SUCC;
}

module_init(AppUiInit);



