#include "app_force.h"

#include "app_manager.h"
#include "app_task_define.h"
#include "init.h"
#include "stdio.h"
#include "queue.h"		
#include "alg.h"

#include "device_manager.h"

#define FORCE_FILTER_FIFO_SIZE 9

static INT16 g_forceFilterData[FORCE_FILTER_FIFO_SIZE] = { 0 };
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
static INT16 g_force = 0;

static INT32 ForceSendData(UINT8 desTag, void *data, UINT16 dataLen)
{
	return SendDataToQueue(TAG_APP_FORCE, desTag, data, dataLen);
}

#define FORCE_CALIBRATION_BUFF_SIZE 16
#define FORCE_CALIBRATION_ONE_SECOND_NUM 30
static INT32 g_forceCalibrationSum = 0;
static bool g_forceCalibration = false;
static void ForceCalibration(INT16 data)
{
	static UINT8 oneSecondCnt = 0;
	static UINT8 index = 0;

	if (++oneSecondCnt > FORCE_CALIBRATION_ONE_SECOND_NUM) {
		oneSecondCnt = 0;
		++index;

		g_forceCalibrationSum += data;
		if (index < FORCE_CALIBRATION_BUFF_SIZE) {
			return;
		}

		g_forceCalibration = false;
		struct sys_config *sysConfig = GetSysConfigOpt()->sysConfig;
		sysConfig->cal.calibratedFlag = 1;
		sysConfig->cal.value = g_forceCalibrationSum / FORCE_CALIBRATION_BUFF_SIZE;
		GetSysConfigOpt()->Write();
		NVIC_SystemReset();
	}
}

static void ForceDeviceProcess(struct platform_info *dev)
{
	struct platform_info *devFops = dev;

	devFops->fops->read(dev);
	INT16 millivolt = *(INT16 *)devFops->private_data;

	if (g_forceCalibration) {
		ForceCalibration(millivolt);
	}

	if (GetSysConfigOpt()->sysConfig->cal.calibratedFlag) {
		millivolt -= GetSysConfigOpt()->sysConfig->cal.value;
	}
	INT16 ret = WeightMovingAverageFilter(&g_forceFilter, millivolt);
	ForceSendData(TAG_APP_WHEEL, &ret, sizeof(ret));
}

static void ForceAppWheelProcess(struct platform_info *app)
{
	g_forceCalibration = true;
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
			case TAG_APP_WHEEL:
				ForceAppWheelProcess(&queueData);
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



