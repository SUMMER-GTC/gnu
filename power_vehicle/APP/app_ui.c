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
#define ENTRY_PARAMETER_CONF_PRESS_CNT 6

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
		UiWriteData(dev, VAR_ICON_START_STOP, STOP_ICON);
	} else {
		UiWriteData(dev, VAR_ICON_START_STOP, START_ICON);
	}

	dev->fops->ioctl(dev, UART_SCREEN_START_TIMER_CMD, (void*)&g_timerRunning, sizeof(g_timerRunning));
}

static UINT8 g_heartLungConnectTimeOutCnt = 0;
static bool g_heartLungWorking = false;
static bool g_startBtn = false;

static UINT8 g_spo2ConnectTimeOutCnt = 0;
static bool g_spo2Connected = false;

static void UiRd800AutoControlProcess(void)
{
	if (!g_heartLungWorking) {
		g_heartLungWorking = true;
		g_startBtn = true;
		StartTimer(true);
	}

	if (g_heartLungWorking) {
		g_heartLungConnectTimeOutCnt = 6;
	}
}

static void UiSpo2AndSoOnUpdate(struct platform_info *dev, struct ui_display_data *uiData)
{
	if (!g_timerRunning) {
		return;
	}

	if (g_dgusPage != PAGE_HEART_LUNG) {
		return;
	}

	if (!g_spo2Connected) {
		g_spo2Connected = true;
	}

	if (g_spo2Connected) {
		g_spo2ConnectTimeOutCnt = 6;
	}

	UiWriteData(dev, DATA_SPO2, uiData->spo2);
	UiWriteData(dev, DATA_VO2, uiData->vo2);
	UiWriteData(dev, DATA_VCO2, uiData->vco2);
	UiWriteData(dev, DATA_HR, uiData->heartRate);
	UiWriteData(dev, DATA_LBP, uiData->lbp);
	UiWriteData(dev, DATA_HBP, uiData->hbp);
}

static void UiSendDataToWheel(UINT8 dataType, UINT16 data)
{
	struct ui_to_wheel uiToWheelData;
	uiToWheelData.dataType = dataType;
	uiToWheelData.data = data;

	UiSendData(TAG_APP_WHEEL, (void *)&uiToWheelData, sizeof(uiToWheelData));
}

static void UiPowerUpdate(struct platform_info *dev, struct ui_display_data *uiData)
{
	if (g_timerRunning) {
		UiWriteData(dev, KEY_RETURN_POWER_INC_DEC, uiData->power);
		UiWriteData(dev, DATA_POWER, uiData->power);
	}
	UiSendData(TAG_APP_DATA_STORAGE, (void *)&uiData->power, sizeof(UINT16));
	UiSendDataToWheel(COMM_POWER_VEHICLE_SET_POWER, uiData->power);
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
			UiRd800AutoControlProcess();
			break;
		case COMM_POWER_VEHICLE_SET_POWER:
			UiPowerUpdate(dev, &g_uiDisplayData);
			break;
		case COMM_POWER_VEHICLE_SPO2_AND_SO_ON:
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
			UiSpo2AndSoOnUpdate(dev, &g_uiDisplayData);
			break;
		default:
			break;
	}
}

static void UiAppRotateSpeedProcess(struct platform_info *app)
{
	struct rotate_speed *speed = (struct rotate_speed *)app->private_data;
	g_uiDisplayData.rpm = speed->displayRpm;
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
	UiSendData(TAG_APP_DATA_STORAGE, (void *)&value, sizeof(UINT16));
	UiSendDataToWheel(KEY_RETURN_POWER_INC_DEC, value);
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
	if (!g_spo2Connected && g_dgusPage == PAGE_HEART_LUNG) {
		g_dgusPage = PAGE_STANDALONE;
		dev->fops->ioctl(dev, DGUS_PAGE_W_CMD, (void*)&g_dgusPage, sizeof(g_dgusPage));
	}

	if (!g_timerRunning) {
		return;
	}

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

	struct platform_info *max6950Dev = NULL;
	if (GetDeviceInfo(TAG_DEVICE_MAX6950, &max6950Dev) != SUCC) {
		return;
	}
	max6950Dev->fops->write(max6950Dev, (void *)&g_uiDisplayData.rpm, sizeof(g_uiDisplayData.rpm));
}

static UINT16 g_entryParameterConfPageBefor = 0;
static struct sys_config g_lastParameterConfig = { 0 };
static void EntryParameterConfProcess(struct platform_info *dev)
{
	static UINT8 pressCnt = 0;

	if (++pressCnt >= ENTRY_PARAMETER_CONF_PRESS_CNT) {
		pressCnt = 0;
		g_entryParameterConfPageBefor = g_dgusPage;
		g_lastParameterConfig = *GetSysConfigOpt()->sysConfig;

		g_dgusPage = PAGE_PARAMETER_CONF;
		dev->fops->ioctl(dev, DGUS_PAGE_W_CMD, (void*)&g_dgusPage, sizeof(g_dgusPage));
	}
}

static void ExitParameterConfProcess(struct platform_info *dev)
{
	struct sys_config *sysConfig = GetSysConfigOpt()->sysConfig;
	if (g_lastParameterConfig.Kcoef != sysConfig->Kcoef || g_lastParameterConfig.Kp != sysConfig->Kp || \
	    g_lastParameterConfig.Ki != sysConfig->Ki || g_lastParameterConfig.Kd != sysConfig->Kd) {
		GetSysConfigOpt()->Write();		
	}

	g_dgusPage = g_entryParameterConfPageBefor;
	dev->fops->ioctl(dev, DGUS_PAGE_W_CMD, (void*)&g_dgusPage, sizeof(g_dgusPage));
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
		case KEY_RETURN_ENTRY_PARAMETER_CONF:
			EntryParameterConfProcess(dev);
			break;
		case KEY_RETURN_EXIT_PARAMETER_CONF:
			ExitParameterConfProcess(dev);
			break;
		case KEY_KCOEF_INC_DEC:
			GetSysConfigOpt()->sysConfig->Kcoef = keyData;
			UiWriteData(dev, DATA_KCOEF, keyData);
			UiSendDataToWheel(KEY_KCOEF_INC_DEC, keyData);
			break;
		case KEY_KP_INC_DEC:
			GetSysConfigOpt()->sysConfig->Kp = keyData;
			UiWriteData(dev, DATA_KP, keyData);
			UiSendDataToWheel(KEY_KP_INC_DEC, keyData);
			break;
		case KEY_KI_INC_DEC:
			GetSysConfigOpt()->sysConfig->Ki = keyData;
			UiWriteData(dev, DATA_KI, keyData);
			UiSendDataToWheel(KEY_KI_INC_DEC, keyData);
			break;
		case KEY_KD_INC_DEC:
			GetSysConfigOpt()->sysConfig->Kd = keyData;
			UiWriteData(dev, DATA_KD, keyData);
			UiSendDataToWheel(KEY_KD_INC_DEC, keyData);
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
		UINT64 freeHeapSize	= xPortGetMinimumEverFreeHeapSize();		
		PrintfLogInfo(DEBUG_LEVEL, "[app_ui][UiTask] queue remain %d, freeHeapSize %d\n", cnt, freeHeapSize);


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
	if (g_heartLungWorking && g_heartLungConnectTimeOutCnt == 0) {
		g_heartLungWorking = false;
		g_startBtn = false;
		StartTimer(false);
	}

	g_spo2ConnectTimeOutCnt = (g_spo2ConnectTimeOutCnt > 0)? (g_spo2ConnectTimeOutCnt - 1): 0;
	if (g_spo2Connected && g_dgusPage == PAGE_HEART_LUNG && g_spo2ConnectTimeOutCnt == 0) {
		g_spo2Connected = false;
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
	GetSysConfigOpt()->Read();

	struct platform_info *dev = NULL;
	if (GetDeviceInfo(TAG_DEVICE_UART_SCREEN, &dev) == SUCC) {
		UiWriteData(dev, DATA_KCOEF, GetSysConfigOpt()->sysConfig->Kcoef);
		UiWriteData(dev, DATA_KP, GetSysConfigOpt()->sysConfig->Kp);
		UiWriteData(dev, DATA_KI, GetSysConfigOpt()->sysConfig->Ki);
		UiWriteData(dev, DATA_KD, GetSysConfigOpt()->sysConfig->Kd);

		UiWriteData(dev, KEY_KCOEF_INC_DEC, GetSysConfigOpt()->sysConfig->Kcoef);
		UiWriteData(dev, KEY_KP_INC_DEC, GetSysConfigOpt()->sysConfig->Kp);
		UiWriteData(dev, KEY_KI_INC_DEC, GetSysConfigOpt()->sysConfig->Ki);
		UiWriteData(dev, KEY_KD_INC_DEC, GetSysConfigOpt()->sysConfig->Kd);
	}

	HeartLungConnectTimer();

	static TaskHandle_t pUiTCB = NULL;
	TaskInit(g_appUi.tag, UiTask, &pUiTCB);
	RegisterApp(&g_appUi);
	return SUCC;
}

module_init(AppUiInit);



