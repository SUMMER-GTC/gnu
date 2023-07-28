#include "app_computer.h"
#include "app_ui.h"

#include "app_manager.h"
#include "app_task_define.h"
#include "init.h"
#include "stdio.h"
#include "queue.h"
#include "device_manager.h"

static INT32 ComputerSendData(UINT8 desTag, void *data, UINT16 dataLen)
{
	return SUCC;
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

static void ComputerDeviceProcessDac()
{
	PrintfLogInfo(DEBUG_LEVEL, "[app_computer][ComputerDeviceProcessDac]\n");

	struct platform_info *optDev = NULL;
	if (GetDeviceInfo(TAG_DEVICE_UPPER_COMPUTER, &optDev) != SUCC) {
		return;
	}
	
	char *str = "app computer\r\n";
	optDev->fops->write(optDev, str, strlen(str));
}

static void ComputerDeviceProcess(struct platform_info *data)
{
	struct platform_info *pData = data;
	switch(pData->tag) {
		case TAG_DEVICE_UPPER_COMPUTER:
			ComputerDeviceProcessDac();
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

static UINT8 g_computerData[COMPUTER_DATA_BUFF_SIZE] = { 0 };

static struct platform_info g_appComputer = {
	.tag = TAG_APP_COMPUTER,
	.private_data = g_computerData,
};

static INT32 AppComputerInit(void)
{
	static TaskHandle_t pComputerTCB = NULL;
	TaskInit(g_appComputer.tag, ComputerTask, &pComputerTCB);
	RegisterApp(&g_appComputer);
	return SUCC;
}

module_init(AppComputerInit);



