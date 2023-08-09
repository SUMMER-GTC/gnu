#include "app_rotate_speed.h"

#include "app_manager.h"
#include "app_task_define.h"
#include "init.h"
#include "stdio.h"
#include "queue.h"		
#include "alg.h"

#define LOW_ROTATE_SPEED_LIMIT 60

#define ROTATE_SPEED_FILTER_FIFO_SIZE 9

static UINT16 g_rotateSpeedFilterData[ROTATE_SPEED_FILTER_FIFO_SIZE] = { 0 };
static struct fifo g_rotateSpeedFifo = {
	.data = g_rotateSpeedFilterData,
	.head = 0,
	.tail = 0
};

static UINT16 g_rotateSpeedFilterWeight[ROTATE_SPEED_FILTER_FIFO_SIZE - 1] = {
	1, 2, 3, 4, 5, 6, 7, 8
};
static struct weight_moving_average_filter g_rotateSpeedFilter = {
	.fifo = &g_rotateSpeedFifo,
	.weight = g_rotateSpeedFilterWeight,
	.fifoSize = ROTATE_SPEED_FILTER_FIFO_SIZE
};
static UINT16 g_rpm = 0;

static INT32 RotateSpeedSendData(UINT8 desTag, void *data, UINT16 dataLen)
{
	return SendDataToQueue(TAG_APP_ROTATE_SPEED, desTag, data, dataLen);
}

static UINT16 RotateSpeedDataFilter(UINT16 rpm)
{
	UINT16 retRPM = 0;
	static UINT16 testRpm = 0;
	++testRpm;
	UINT16 ret = WeightMovingAverageFilter(&g_rotateSpeedFilter, 10);
	RotateSpeedSendData(TAG_APP_WHEEL, &ret, sizeof(ret));

	if (rpm < LOW_ROTATE_SPEED_LIMIT) {
		retRPM = rpm;
	}

	return retRPM;
}

static void RotateSpeedDeviceProcess(struct platform_info *dev)
{
	struct platform_info *devFops = dev;

	devFops->fops->read(dev);
	UINT16 rpm = *(UINT16 *)devFops->private_data;

	RotateSpeedDataFilter(rpm);
}

void RotateSpeedTask(void *pvParameters)
{
	struct platform_info queueData;
	QueueHandle_t xQueue = (QueueHandle_t)pvParameters;
	for(;;) {
		xQueueReceive(xQueue, &queueData, portMAX_DELAY);

		UINT8 cnt = uxQueueSpacesAvailable(xQueue);
		PrintfLogInfo(DEBUG_LEVEL, "[app_rotate_speed][RotateSpeedTask] queue remain %d\n", cnt);

		RotateSpeedDeviceProcess(&queueData);
	}

}

static struct platform_info g_appRotateSpeed = {
	.tag = TAG_APP_ROTATE_SPEED,
	.private_data = &g_rpm,
	.private_data_len = sizeof(g_rpm),
};

static INT32 AppRotateSpeedInit(void)
{
	static TaskHandle_t protateSpeedTCB = NULL;
	TaskInit(g_appRotateSpeed.tag, RotateSpeedTask, &protateSpeedTCB);
	RegisterApp(&g_appRotateSpeed);
	return SUCC;
}

module_init(AppRotateSpeedInit);



