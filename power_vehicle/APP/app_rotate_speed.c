#include "app_rotate_speed.h"

#include "app_manager.h"
#include "app_task_define.h"
#include "init.h"
#include "stdio.h"
#include "queue.h"		

static void RotateSpeedDeviceProcess(struct platform_info *dev)
{
	struct platform_info *devFops = dev;

	devFops->fops->read(dev);
}

void RotateSpeedTask(void *pvParameters)
{
	struct platform_info queueData;
	QueueHandle_t xQueue = (QueueHandle_t)pvParameters;
	for(;;) {
		xQueueReceive(xQueue, &queueData, portMAX_DELAY);
		
		RotateSpeedDeviceProcess(&queueData);
	}

}

static struct platform_info g_appRotateSpeed = {
	.tag = TAG_APP_ROTATE_SPEED,
};

static INT32 AppRotateSpeedInit(void)
{
	static TaskHandle_t protateSpeedTCB = NULL;
	TaskInit(g_appRotateSpeed.tag, RotateSpeedTask, &protateSpeedTCB);
	RegisterApp(&g_appRotateSpeed);
	return SUCC;
}

module_init(AppRotateSpeedInit);



