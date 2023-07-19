#include "app_therapy.h"
#include "app_ui.h"

#include "app_manager.h"
#include "app_task_define.h"
#include "init.h"
#include "stdio.h"
#include "queue.h"		

static INT32 TherapySendData(UINT8 desTag, void *data, UINT16 dataLen)
{
	return SUCC;
	return SendDataToQueue(TAG_APP_THERAPY, desTag, data, dataLen);
}

static void TherapyAppProcessUi(struct platform_info *data)
{
	UINT8 recData = *((UINT8*)data->private_data);
	UINT16 recDataLen = data->private_data_len;
	PrintfLogInfo(DEBUG_LEVEL, "[app_therapy][TherapyAppProcessUi] data=%d, dataLen=%d\n", \
						 recData, recDataLen);

}

static void TherapyAppProcess(struct platform_info *data)
{
	struct platform_info *pData = data;
	switch(pData->tag) {
		case TAG_APP_UI:
			TherapyAppProcessUi(pData);
			break;
		default:
		
			break;
	}
}

static void TherapyDeviceProcessDac()
{
	PrintfLogInfo(DEBUG_LEVEL, "[app_therapy][TherapyDeviceProcessDac]\n");
	UINT8 therapySendData = 22;
	TherapySendData(TAG_APP_UI, &therapySendData, sizeof(therapySendData));
}

static void TherapyDeviceProcess(struct platform_info *data)
{
	return;
	struct platform_info *pData = data;
	switch(pData->tag) {
		case 0:
			TherapyDeviceProcessDac();
			break;
		default:
		
			break;
	}
}

static void TherapyTask(void *pvParameters)
{
	struct platform_info queueData;
	QueueHandle_t xQueue = (QueueHandle_t)pvParameters;
	for(;;) {
		xQueueReceive(xQueue, &queueData, portMAX_DELAY);
		
		UINT8 cnt = uxQueueSpacesAvailable(xQueue);
		PrintfLogInfo(DEBUG_LEVEL, "[app_therapy][TherapyTask] queue remain %d\n", cnt);

		if (queueData.tag < TAG_APP_END) {
			TherapyAppProcess(&queueData);
		} else if (queueData.tag >= TAG_DEVICE_START && queueData.tag < TAG_DEVICE_END) {
			TherapyDeviceProcess(&queueData);
		} else {
			// unknow tag error
		}
	}
}

static UINT8 g_therapyData[THERAPY_DATA_BUFF_SIZE] = { 0 };

static struct platform_info g_appTherapy = {
	.tag = TAG_APP_THERAPY,
	.private_data = g_therapyData,
};

static INT32 AppTherapyInit(void)
{
	static TaskHandle_t pTherapyTCB = NULL;
	TaskInit(g_appTherapy.tag, TherapyTask, &pTherapyTCB);
	RegisterApp(&g_appTherapy);
	return SUCC;
}

module_init(AppTherapyInit);



