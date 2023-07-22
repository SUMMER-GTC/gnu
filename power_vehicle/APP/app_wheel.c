#include "app_wheel.h"

#include "app_manager.h"
#include "app_task_define.h"
#include "init.h"
#include "stdio.h"
#include "queue.h"		

__packed struct wheel_control {
	UINT16 force;
	UINT16 rpm;
	UINT16 power;
};

static struct wheel_control g_wheelControl = { 0 };

static void WheelAppRotateSpeedProcess(struct platform_info *dev)
{
	g_wheelControl.rpm = *(UINT16 *)dev->private_data;
	g_wheelControl.force = g_wheelControl.power / g_wheelControl.rpm;
	
}

static void WheelProcess(struct platform_info *dev)
{
	switch (dev->tag) {
		case TAG_APP_FORCE:
			g_wheelControl.force = *(UINT16 *)dev->private_data;
			break;
		case TAG_APP_ROTATE_SPEED:
			WheelAppRotateSpeedProcess(dev);
			break;
		case TAG_DEVICE_UART_SCREEN:
			g_wheelControl.power = 10;
			// g_wheelControl.power = *(UINT16 *)dev->private_data;
			break;
		default:
			break;
	}
}

void WheelTask(void *pvParameters)
{
	struct platform_info queueData;
	QueueHandle_t xQueue = (QueueHandle_t)pvParameters;
	for(;;) {
		xQueueReceive(xQueue, &queueData, portMAX_DELAY);
		
		WheelProcess(&queueData);
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



