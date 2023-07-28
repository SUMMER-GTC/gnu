#include "device_manager.h"
#include "app_task_define.h"
#include "timers.h"

static struct platform_info *g_deviceInfo[TAG_DEVICE_END - TAG_DEVICE_START];

#define SET_INTERVAL_TICK 2
#define SET_INTERVAL_BUFF_SIZE (TAG_DEVICE_END - TAG_DEVICE_START)

struct dev_set_interval_array {
	UINT16 timeCnt;
	UINT16 DevSetInterval;
	bool useFlag;
	void (* DeviceSetIntervalCall)(void *);
};

struct dev_set_interval_array g_devSetInterval[SET_INTERVAL_BUFF_SIZE];
static TimerHandle_t g_setIntervalTimerHandle = NULL;

static void DeviceManagerSetInterval(UINT8 tag, UINT16 interval, void (* fn)(void *))
{
	UINT8 index = tag - TAG_DEVICE_START;
	g_devSetInterval[index].DevSetInterval = interval;
	g_devSetInterval[index].DeviceSetIntervalCall = fn;
	g_devSetInterval[index].useFlag = true;
}

static void SetIntervalTimerCall(void)
{
	for (UINT8 i = 0; i < SET_INTERVAL_BUFF_SIZE; i++) {
		if (g_devSetInterval[i].useFlag != true || g_devSetInterval[i].DeviceSetIntervalCall == NULL) {
			continue;
		}
		g_devSetInterval[i].timeCnt += SET_INTERVAL_TICK;
		if (g_devSetInterval[i].timeCnt >= g_devSetInterval[i].DevSetInterval) {
			g_devSetInterval[i].timeCnt = 0;
			g_devSetInterval[i].DeviceSetIntervalCall(g_deviceInfo[i]);
		}
	}
}

INT32 DeviceCreateSetIntervalTimer(void)
{
	g_setIntervalTimerHandle = xTimerCreate(
																"DeviceSetIntervalTimer",
																pdMS_TO_TICKS(SET_INTERVAL_TICK),
																pdTRUE,
																0,
																(TimerCallbackFunction_t)SetIntervalTimerCall);
	if (g_setIntervalTimerHandle == NULL) {
		return FAIL;
	}

	xTimerStart(g_setIntervalTimerHandle, 0);
	return SUCC;
}

void RegisterDevice(struct platform_info *dev)
{
	if (dev == NULL) {
		return;
	}

	if (dev->tag < TAG_DEVICE_START || dev->tag >= TAG_DEVICE_END) {
		return;
	}
	
	g_deviceInfo[dev->tag - TAG_DEVICE_START] = dev;

	if (dev->IntervalCall == NULL || dev->setInterval == 0) {
		PrintfLogInfo(DEBUG_LEVEL, "[device_manager][DeviceManagerSetInterval] tag=%d unuse set interval\n", dev->tag);
		return;
	}

	PrintfLogInfo(DEBUG_LEVEL, "[device_manager][DeviceManagerSetInterval] tag=%d use set interval\n", dev->tag);
	DeviceManagerSetInterval(dev->tag, dev->setInterval, dev->IntervalCall);
}

INT32 GetDeviceInfo(UINT8 tag, struct platform_info **dev)
{
	if (dev == NULL) {
		return FAIL;
	}
	
	if (tag < TAG_DEVICE_START || tag > TAG_DEVICE_END) {
		return FAIL;
	}

	*dev = g_deviceInfo[tag - TAG_DEVICE_START];

	return SUCC;
}

INT32 DeviceSampleData(UINT8 isrFlag, UINT8 tag, struct platform_info *data)
{
	return SendDataToApp(isrFlag, tag, data);
}


