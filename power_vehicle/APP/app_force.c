#include "app_force.h"

#include "app_manager.h"
#include "app_task_define.h"
#include "init.h"
#include "stdio.h"
#include "queue.h"		
#include "alg.h"

#define FORCE_FILTER_FIFO_SIZE 9

static UINT16 g_forceFilterData[FORCE_FILTER_FIFO_SIZE] = { 0 };
static struct fifo g_forceFifo = {
	.data = g_forceFilterData,
	.head = 0,
	.tail = 0
};

static UINT16 g_forceFilterWeight[FORCE_FILTER_FIFO_SIZE - 1] = {
	1, 2, 3, 4, 5, 6, 7, 8
};
static struct weight_moving_average_filter g_forceFilter = {
	.fifo = &g_forceFifo,
	.weight = g_forceFilterWeight,
	.fifoSize = FORCE_FILTER_FIFO_SIZE
};
static UINT16 g_force = 0;

static INT32 ForceSendData(UINT8 desTag, void *data, UINT16 dataLen)
{
	return SendDataToQueue(TAG_APP_FORCE, desTag, data, dataLen);
}

static void ForceDeviceProcess(struct platform_info *dev)
{
	struct platform_info *devFops = dev;

	devFops->fops->read(dev);
	UINT16 millivolt = *(UINT16 *)devFops->private_data;
	UINT16 ret = WeightMovingAverageFilter(&g_forceFilter, millivolt);

	ForceSendData(TAG_APP_WHEEL, &ret, sizeof(ret));
}

void ForceTask(void *pvParameters)
{
	struct platform_info queueData;
	QueueHandle_t xQueue = (QueueHandle_t)pvParameters;
	for(;;) {
		xQueueReceive(xQueue, &queueData, portMAX_DELAY);

		UINT8 cnt = uxQueueSpacesAvailable(xQueue);
		PrintfLogInfo(DEBUG_LEVEL, "[app_force][ForceTask] queue remain %d\n", cnt);

		switch (queueData.tag) {
			case TAG_DEVICE_TASEOMETER:
				ForceDeviceProcess(&queueData);
				break;
		}
	}

}

static struct platform_info g_appForce = {
	.tag = TAG_APP_FORCE,
	.private_data = &g_force,
	.private_data_len = sizeof(g_force),
};

static INT32 AppForceInit(void)
{
	static TaskHandle_t pforceTCB = NULL;
	TaskInit(g_appForce.tag, ForceTask, &pforceTCB);
	RegisterApp(&g_appForce);
	return SUCC;
}

module_init(AppForceInit);



