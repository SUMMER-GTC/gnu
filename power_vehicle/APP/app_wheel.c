#include "app_wheel.h"

#include "app_manager.h"
#include "app_task_define.h"
#include "init.h"
#include "stdio.h"
#include "queue.h"		

static void WheelDeviceProcess(struct platform_info *dev)
{
	struct platform_info *devFops = dev;

	UINT32 duty = 750;
	devFops->fops->write(dev, (void *)&duty, sizeof(duty));
}

void WheelTask(void *pvParameters)
{
	struct platform_info queueData;
	QueueHandle_t xQueue = (QueueHandle_t)pvParameters;
	for(;;) {
		xQueueReceive(xQueue, &queueData, portMAX_DELAY);
		
		WheelDeviceProcess(&queueData);
	}

}

static struct platform_info g_appWheel = {
	.tag = TAG_APP_WHEEL,
};

static INT32 AppWheelInit(void)
{
	static TaskHandle_t pwheelTCB = NULL;
	TaskInit(g_appWheel.tag, WheelTask, &pwheelTCB);
	RegisterApp(&g_appWheel);
	return SUCC;
}

module_init(AppWheelInit);



