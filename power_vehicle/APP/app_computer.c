#include "app_computer.h"
#include "app_ui.h"

#include "app_manager.h"
#include "app_task_define.h"
#include "init.h"
#include "stdio.h"
#include "queue.h"
#include "device_manager.h"
#include "chip_flash.h"
#include "timers.h"

static char g_powerVehicleDeviceId[] = "0900P10V757";

static UINT8 g_computerData[COMPUTER_DATA_BUFF_SIZE] = { 0 };

static INT32 ComputerSendData(UINT8 desTag, void *data, UINT16 dataLen)
{
	return SendDataToQueue(TAG_APP_COMPUTER, desTag, data, dataLen);
}

static void ComputerAppProcessUi(struct platform_info *data)
{
	UINT8 recData = *((UINT8*)data->private_data);
	UINT16 recDataLen = data->private_data_len;
	PrintfLogInfo(DEBUG_LEVEL, "[app_computer][ComputerAppProcessUi] data=%d, dataLen=%d\n", \
						 recData, recDataLen);

}

static void ComputerAppProcess(struct platform_info *data)
{
	struct platform_info *pData = data;
	switch(pData->tag) {
		case TAG_APP_UI:
			ComputerAppProcessUi(pData);
			break;
		default:
		
			break;
	}
}

static void RD800DataSimulator(struct platform_info *dev)
{
	struct ui_display_data *uiData = (struct ui_display_data *)g_computerData;
	uiData->rpm = rand() % 1000;
	uiData->power = rand() % 1000;
	uiData->d1 = rand() % 1000;
	uiData->d2 = rand() % 1000;
	uiData->d3 = rand() % 1000;
	uiData->d4 = rand() % 1000;
	uiData->d5 = rand() % 1000;
	ComputerSendData(TAG_APP_UI, uiData, sizeof(struct ui_display_data));
}

static UINT8 g_rd800SetPower = 0;
static void RD800Process(struct platform_info *dev)
{
	struct upper_computer *upperComputer = (struct upper_computer *)dev->private_data;
	struct ui_display_data *uiData = (struct ui_display_data *)g_computerData;	
	UINT8 *powerVehicle = upperComputer->rd800;
	char sendData[5] = "n000";

	switch(powerVehicle[0]) {
		case COMM_POWER_VEHICLE_CONNECT:
			dev->fops->write(dev, g_powerVehicleDeviceId, strlen(g_powerVehicleDeviceId));
			break;
		case COMM_POWER_VEHICLE_READ_SPEED:
			itoa(uiData->rpm % 1000, &sendData[1], 10);
			dev->fops->write(dev, sendData, strlen(sendData));
			break;
		case COMM_POWER_VEHICLE_SET_POWER:
			char *numStr = (char *)&powerVehicle[1];
			g_rd800SetPower = atoi(numStr);
		break;
		default: break;
	}
}

#define COMPUTER_OTA_SEND_BUFF_SIZE (32)
static UINT8 g_sendBuff[COMPUTER_OTA_SEND_BUFF_SIZE] = { 0 };
static UINT16 g_sendCnt = 0;
static bool OtaReplyProcess(struct platform_info *dev, struct ota_protocol *pOta)
{
	UINT8 *dataPtr = (UINT8 *)pOta;
	UINT16 dataLen = COMM_OTA_OFFSET(data);

 	if (dataLen > COMPUTER_OTA_SEND_BUFF_SIZE) {
		dataLen = COMPUTER_OTA_SEND_BUFF_SIZE;
	}
	
	for (UINT16 i = 0; i < dataLen; i++) {
		g_sendBuff[i] = dataPtr[i];
	}

	dev->fops->write(dev, g_sendBuff, dataLen);
	++g_sendCnt;

  return true;
}

static INT32 OtaRunBootloader(struct platform_info *dev, struct ota_protocol *pOta)
{
  GetSysConfigOpt()->Read();
	GetSysConfigOpt()->sysConfig->otaState = OTA_RUN_BOOTLOADER;
	GetSysConfigOpt()->Write();
  OtaReplyProcess(dev, pOta);
	NVIC_SystemReset();
	return SUCC;
}

static INT32 OtaProcess(struct platform_info *dev)
{
	struct upper_computer *upperComputer = (struct upper_computer *)dev->private_data;
	struct ota_protocol *pOta = upperComputer->ota;
	
	if (pOta->subCommand >= OTA_SUB_COMMAND_END) {
		return FAIL;
	}
	
	switch(pOta->subCommand) {
		case OTA_RUN_BOOTLOADER:
			OtaRunBootloader(dev, pOta);
			break;
		default:
			break;
	}

	return SUCC;
}

static void UpperComputerDeviceProcess(struct platform_info *dev)
{
	struct upper_computer *pUpperComputer = (struct upper_computer *)dev->private_data;

	switch(pUpperComputer->dataType) {
  	case UPPER_COMPUTER_RD800_DATA:
			RD800Process(dev);
			break;
  	case UPPER_COMPUTER_OTA_DATA:
			OtaProcess(dev);
			break;
		default:
			break;
	}
}

static void ComputerDeviceProcess(struct platform_info *dev)
{
	switch(dev->tag) {
		case TAG_DEVICE_UPPER_COMPUTER:
			UpperComputerDeviceProcess(dev);
			break;
		default:
		
			break;
	}
}

static void ComputerTask(void *pvParameters)
{
	struct platform_info queueData;
	QueueHandle_t xQueue = (QueueHandle_t)pvParameters;
	for(;;) {
		xQueueReceive(xQueue, &queueData, portMAX_DELAY);
		
		UINT8 cnt = uxQueueSpacesAvailable(xQueue);
		PrintfLogInfo(DEBUG_LEVEL, "[app_computer][ComputerTask] queue remain %d\n", cnt);

		if (queueData.tag < TAG_APP_END) {
			ComputerAppProcess(&queueData);
		} else if (queueData.tag >= TAG_DEVICE_START && queueData.tag < TAG_DEVICE_END) {
			ComputerDeviceProcess(&queueData);
		} else {
			// unknow tag error
		}
	}
}

static struct platform_info g_appComputer = {
	.tag = TAG_APP_COMPUTER,
	.private_data = g_computerData,
	.private_data_len = sizeof(g_computerData),
};

static TimerHandle_t g_simulatorUiDataHandle = NULL;

static void SimulatorUiDataTimerCall(void)
{
	RD800DataSimulator(&g_appComputer);
}

static INT32 SimulatorUiDataTimer(void)
{
	g_simulatorUiDataHandle = xTimerCreate(
																"SimulatorUiDataTimer",
																pdMS_TO_TICKS(1000),
																pdTRUE,
																0,
																(TimerCallbackFunction_t)SimulatorUiDataTimerCall);
	if (g_simulatorUiDataHandle == NULL) {
		return FAIL;
	}

	xTimerStart(g_simulatorUiDataHandle, 0);
	return SUCC;
}

static INT32 AppComputerInit(void)
{
	SimulatorUiDataTimer();

	static TaskHandle_t pComputerTCB = NULL;
	TaskInit(g_appComputer.tag, ComputerTask, &pComputerTCB);
	RegisterApp(&g_appComputer);
	return SUCC;
}

module_init(AppComputerInit);



