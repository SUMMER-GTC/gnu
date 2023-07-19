#include "app_force.h"

#include "app_manager.h"
#include "app_task_define.h"
#include "init.h"
#include "stdio.h"
#include "queue.h"		

static void ForceDeviceProcess(struct platform_info *dev)
{
	struct platform_info *devFops = dev;

	devFops->fops->read(dev);
}

void ForceTask(void *pvParameters)
{
	struct platform_info queueData;
	QueueHandle_t xQueue = (QueueHandle_t)pvParameters;
	for(;;) {
		xQueueReceive(xQueue, &queueData, portMAX_DELAY);
		
		ForceDeviceProcess(&queueData);
	}

}

static struct platform_info g_appForce = {
	.tag = TAG_APP_FORCE,
};

static INT32 AppForceInit(void)
{
	static TaskHandle_t pforceTCB = NULL;
	TaskInit(g_appForce.tag, ForceTask, &pforceTCB);
	RegisterApp(&g_appForce);
	return SUCC;
}

module_init(AppForceInit);



