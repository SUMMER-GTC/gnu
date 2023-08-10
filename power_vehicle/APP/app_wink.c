#include "app_wink.h"

#include "app_manager.h"
#include "app_task_define.h"
#include "init.h"
#include "stdio.h"
#include "queue.h"		

static void WinkDeviceProcess(struct platform_info *dev)
{
	static UINT8 cnt = 0;
	struct platform_info *devFops = dev;

	++cnt;
	UINT8 bitValue = cnt % 2;
	devFops->fops->write(dev, (void *)&bitValue, sizeof(bitValue));
}

void WinkTask(void *pvParameters)
{
	struct platform_info queueData;
	QueueHandle_t xQueue = (QueueHandle_t)pvParameters;
	for(;;) {
		xQueueReceive(xQueue, &queueData, portMAX_DELAY);
		
		switch (queueData.tag) { 
			case TAG_DEVICE_LED:
				WinkDeviceProcess(&queueData);			
				break;
		}
	}

}

static struct platform_info g_appWink = {
	.tag = TAG_APP_WINK,
};

static INT32 AppWinkInit(void)
{
	static TaskHandle_t pwinkTCB = NULL;
	TaskInit(g_appWink.tag, WinkTask, &pwinkTCB);
	RegisterApp(&g_appWink);
	return SUCC;
}

module_init(AppWinkInit);



