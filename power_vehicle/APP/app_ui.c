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
static struct ui_display_data g_uiDisplayData;

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

static UINT16 g_timerSecond = 0;
static bool g_timerRunning = false;

static void StartTimer(bool startRun)
{
	struct platform_info *dev;
	if (GetDeviceInfo(TAG_DEVICE_UART_SCREEN, &dev) == FAIL) {
		return;
	}

	if (startRun == g_timerRunning) {
		return;
	}

	g_timerRunning = startRun;

	if (g_timerRunning) {
		g_timerSecond = 0;
		UiWriteData(dev, DATA_TIME, g_timerSecond);
	}

	dev->fops->ioctl(dev, UART_SCREEN_START_TIMER_CMD, (void*)&g_timerRunning, sizeof(g_timerRunning));
}

static UINT8 g_heartLungConnectTimeOutCnt = 0;
static bool g_heartLungWorking = false;
static bool g_startBtn = false;

static void UiUpdate(struct platform_info *dev, struct ui_display_data *uiData)
{
	if (!g_heartLungWorking) {
		g_heartLungWorking = true;
		g_startBtn = true;
		StartTimer(true);
	}

	if (g_heartLungWorking) {
		g_heartLungConnectTimeOutCnt = 6;
	}

	if (g_timerRunning) {
		UiWriteData(dev, DATA_SPO2, uiData->spo2);
		UiWriteData(dev, DATA_VO2, uiData->vo2);
		UiWriteData(dev, DATA_VCO2, uiData->vco2);
		UiWriteData(dev, DATA_HR, uiData->heartRate);
		UiWriteData(dev, DATA_LBP, uiData->lbp);
		UiWriteData(dev, DATA_HBP, uiData->hbp);		
	}
}

static void UiPowerUpdate(struct platform_info *dev, struct ui_display_data *uiData)
{
	if (g_timerRunning) {
		UiWriteData(dev, KEY_RETURN_POWER_INC_DEC, uiData->power);
		UiWriteData(dev, DATA_POWER, uiData->power);
	}
}

static bool g_logDelayOver = false;
static void UiAppComputerProcess(struct platform_info *app)
{
	if (!g_logDelayOver) {
		return;
	}

	struct ui_display_data *uiData = (struct ui_display_data *)app->private_data;
	UINT16 rpm = g_uiDisplayData.rpm;
	memcpy(&g_uiDisplayData, uiData, sizeof(struct ui_display_data));
	g_uiDisplayData.rpm = rpm;

	struct platform_info *dev;
	if (GetDeviceInfo(TAG_DEVICE_UART_SCREEN, &dev) == FAIL) {
		return;
	}

	switch(g_uiDisplayData.dataType) {
		case COMM_POWER_VEHICLE_CONNECT:
			break;
		case COMM_POWER_VEHICLE_READ_SPEED:
			if (g_dgusPage != PAGE_HEART_LUNG) {
				UiWriteData(dev, DATA_SPO2, 0);
				UiWriteData(dev, DATA_VO2, 0);
				UiWriteData(dev, DATA_VCO2, 0);
				UiWriteData(dev, DATA_HR, 0);
				UiWriteData(dev, DATA_LBP, 0);
				UiWriteData(dev, DATA_HBP, 0);
				g_dgusPage = PAGE_HEART_LUNG;
				dev->fops->ioctl(dev, DGUS_PAGE_W_CMD, (void*)&g_dgusPage, sizeof(g_dgusPage));
			}
			UiUpdate(dev, &g_uiDisplayData);
			break;
		case COMM_POWER_VEHICLE_SET_POWER:
			UiPowerUpdate(dev, &g_uiDisplayData);
			break;
		default:
			break;
	}
}

static void UiAppRotateSpeedProcess(struct platform_info *app)
{
	g_uiDisplayData.rpm = *(UINT16 *)app->private_data;
}

static void UiAppProcess(struct platform_info *data)
{
	switch(data->tag) {
		case TAG_APP_COMPUTER:
			UiAppComputerProcess(data);
			break;
		case TAG_APP_ROTATE_SPEED:
			UiAppRotateSpeedProcess(data);
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

static void StartProcess(struct platform_info *dev)
{
	g_startBtn = !g_startBtn;
	StartTimer(g_startBtn);
}

static void UiLogDelayProcess(struct platform_info *dev)
{
	// change to home page 
	g_logDelayOver = true;
	g_dgusPage = PAGE_STANDALONE;
	dev->fops->ioctl(dev, DGUS_PAGE_W_CMD, (void*)&g_dgusPage, sizeof(g_dgusPage));
}

static void UiTimerProcess(struct platform_info *dev)
{
	++g_timerSecond;
	UiWriteData(dev, DATA_TIME, g_timerSecond);

	static UINT16 lastRpm = 0;

	if (g_uiDisplayData.rpm == lastRpm) { 
		return;
	}

	UiWriteData(dev, DATA_RPM, g_uiDisplayData.rpm);
	// change font color
	if (g_uiDisplayData.rpm <= RMP_WARNING_VALUE && lastRpm > RMP_WARNING_VALUE) {
		UiWriteData(dev, DATA_RPM_SP_COLOR, DATA_RPM_RED);
		UiWriteData(dev, DATA_RPM_SP_COLOR, DATA_RPM_RED);
	} else if (g_uiDisplayData.rpm > RMP_WARNING_VALUE && lastRpm <= RMP_WARNING_VALUE) {
		UiWriteData(dev, DATA_RPM_SP_COLOR, DATA_RPM_GREEN);
		UiWriteData(dev, DATA_RPM_SP_COLOR, DATA_RPM_GREEN);
	}

	lastRpm = g_uiDisplayData.rpm;
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
		UINT64 freeHeapSize	= xPortGetMinimumEverFreeHeapSize();

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
static TimerHandle_t g_heartLungHandle = NULL;

static void HeartLungConnectTimerCall(void)
{
	g_heartLungConnectTimeOutCnt = (g_heartLungConnectTimeOutCnt > 0)? (g_heartLungConnectTimeOutCnt - 1): 0;
	if (g_heartLungWorking && g_dgusPage == PAGE_HEART_LUNG && g_heartLungConnectTimeOutCnt == 0) {
		g_heartLungWorking = false;
		g_startBtn = false;
		StartTimer(false);
	}
}

static INT32 HeartLungConnectTimer(void)
{
	g_heartLungHandle = xTimerCreate(
																"HeartLungConnectTimer",
																pdMS_TO_TICKS(1000),
																pdTRUE,
																0,
																(TimerCallbackFunction_t)HeartLungConnectTimerCall);
	if (g_heartLungHandle == NULL) {
		return FAIL;
	}

	xTimerStart(g_heartLungHandle, 0);
	return SUCC;
}

static struct platform_info g_appUi = {
	.tag = TAG_APP_UI,
	.private_data = g_uiData,
};

static INT32 AppUiInit(void)
{
	HeartLungConnectTimer();

	static TaskHandle_t pUiTCB = NULL;
	TaskInit(g_appUi.tag, UiTask, &pUiTCB);
	RegisterApp(&g_appUi);
	return SUCC;
}

module_init(AppUiInit);



