#include "app_ui.h"
#include "app_therapy.h"

#include "app_manager.h"
#include "app_task_define.h"
#include "init.h"
#include "stdio.h"
#include "queue.h"		

static INT32 UiSendData(UINT8 desTag, void *data, UINT16 dataLen)
{
	return SUCC;
	return SendDataToQueue(TAG_APP_UI, desTag, data, dataLen);
}

static void UiAppProcessUi(struct platform_info *data)
{
	UINT8 recData = *((UINT8*)data->private_data);
	UINT16 recDataLen = data->private_data_len;
	PrintfLogInfo(DEBUG_LEVEL, "[app_ui][UiAppProcessUi] data=%d, dataLen=%d\n", \
						 recData, recDataLen);
}

static void UiAppProcess(struct platform_info *data)
{
	switch(data->tag) {
		case TAG_APP_THERAPY:
			UiAppProcessUi(data);
			break;
		default:
		
			break;
	}
}

static void UiDeviceProcessUartScreen(struct platform_info *data)
{
	struct platform_info *devFops = data;

	devFops->fops->read(data);
	
	UINT8* pdate = (UINT8 *)data->private_data;
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

static INT32 UiInit(void *dev)
{
	PrintfLogInfo(DEBUG_LEVEL, "[app_ui][UiInit]...\n");

	return SUCC;
}

static struct file_operations g_uiFops = {
	.init = UiInit,
};

static UINT8 g_uiData[THERAPY_DATA_BUFF_SIZE] = { 0 };

static struct platform_info g_appUi = {
	.tag = TAG_APP_UI,
	.fops = &g_uiFops,
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



