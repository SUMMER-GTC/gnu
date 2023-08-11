#include "app_task_define.h"

#include "common_def.h"
#include "queue.h"
#include "stdlib.h"

#define QUEUE_ITEM_SIZE sizeof(struct platform_info)

static struct app_define g_appDefine[TAG_APP_END] = {
		{
			.tag = TAG_APP_UI,
			.task = {
			 	NULL,
			 	"UiTask",
			 	TASK_STASK_SIZE_BYTES_256,
			 	NULL,
			 	TASK_PRIORITY_HIGH,
			 	NULL
			},
			.queue = {
				10,
				QUEUE_ITEM_SIZE	
			},
		},

		{
			.tag = TAG_APP_COMPUTER,
			.task = {
				NULL,
				"ComputerTask",
				TASK_STASK_SIZE_BYTES_256,
				NULL,
				TASK_PRIORITY_NORMAL,
				NULL
			},
			.queue = {
				10,
				QUEUE_ITEM_SIZE 
			}
		},

		{
			.tag = TAG_APP_WINK,
			.task = {
				NULL,
				"WinkTask",
				TASK_STASK_SIZE_BYTES_64,
				NULL,
				TASK_PRIORITY_LOW,
				NULL
			},
			.queue = {
				2,
				QUEUE_ITEM_SIZE 
			}
		},

		{
			.tag = TAG_APP_WHEEL,
			.task = {
				NULL,
				"WheelTask",
				TASK_STASK_SIZE_BYTES_256,
				NULL,
				TASK_PRIORITY_HIGH,
				NULL
			},
			.queue = {
				10,
				QUEUE_ITEM_SIZE 
			}
		},

		{
			.tag = TAG_APP_ROTATE_SPEED,
			.task = {
				NULL,
				"RotateSpeedTask",
				TASK_STASK_SIZE_BYTES_256,
				NULL,
				TASK_PRIORITY_BELOW_NORMAL,
				NULL
			},
			.queue = {
				10,
				QUEUE_ITEM_SIZE 
			}
		},

		{
			.tag = TAG_APP_FORCE,
			.task = {
				NULL,
				"ForceTask",
				TASK_STASK_SIZE_BYTES_256,
				NULL,
				TASK_PRIORITY_NORMAL,
				NULL
			},
			.queue = {
				10,
				QUEUE_ITEM_SIZE 
			}
		},

		{
			.tag = TAG_APP_DATA_STORAGE,
			.task = {
				NULL,
				"DataStorageTask",
				TASK_STASK_SIZE_BYTES_1024,
				NULL,
				TASK_PRIORITY_ABOVE_NORMAL,
				NULL
			},
			.queue = {
				10,
				QUEUE_ITEM_SIZE 
			}
		},
};

static INT32 TaskQueueCreate(void)
{
	QueueHandle_t pQueue = NULL;
	for (UINT8 i = 0; i < TAG_APP_END; i++) {
		pQueue = xQueueCreate(g_appDefine[i].queue.uxQueueLength, g_appDefine[i].queue.uxItemSize);
		if (pQueue != NULL) {
			g_appDefine[i].task.pvParameters = pQueue;
		} else {
			// printf queue create error information
		}
	}
	return SUCC;
}

INT32 TaskInit(UINT8 tag, TaskFunction_t pfn, TaskHandle_t *pTCB)
{
	if (tag >= TAG_APP_END) {
		return FAIL;
	}

	g_appDefine[tag].task.pxTaskCode = pfn;
	g_appDefine[tag].task.pxCreatedTask = pTCB;

	return SUCC;
}

INT32 TaskCreate(void)
{
	BaseType_t xRet = pdPASS;
	TaskQueueCreate();
	for (UINT8 i = 0; i < TAG_APP_END; i++) {
		xRet = xTaskCreate(
							g_appDefine[i].task.pxTaskCode,
							g_appDefine[i].task.pcName,
							g_appDefine[i].task.usStackDepth,
							g_appDefine[i].task.pvParameters,
							g_appDefine[i].task.uxPriority,
							g_appDefine[i].task.pxCreatedTask);
		if (xRet != pdPASS) {
			PrintfLogInfo(DEBUG_LEVEL, "[app_task_define][TaskCreate] fail, tag=%d\n", i);
		}
	}
	return SUCC;
}

INT32 SendDataToApp(UINT8 isrFlag, UINT8 tag, struct platform_info *data)
{
	if (tag >= TAG_APP_END) {
		return FAIL;
	}

	if (xTaskGetSchedulerState() != taskSCHEDULER_RUNNING) { 
		return FAIL;
	}

	QueueHandle_t xQueue = g_appDefine[tag].task.pvParameters;
	
	BaseType_t ret = pdPASS;
	if (isrFlag == SEND_FROM_ISR) {
		ret = xQueueSendFromISR(xQueue, data, 0);
	} else {
		ret = xQueueSend(xQueue, data, 0);
	}
	if (ret != pdPASS) {
		return FAIL; 
	}
	return SUCC;	
}

static char buff[256] = { 0 };
void TaskRemainStack(void)
{
  UINT64 remainStackSize = 0;
	INT32 writeNum = 0;
	char *pBuff = buff;

	writeNum = sprintf(pBuff, "task remain stack size:\n");
	pBuff += writeNum;

	for (UINT8 i = 0; i < TAG_APP_END; i++) {
		remainStackSize = uxTaskGetStackHighWaterMark(*g_appDefine[i].task.pxCreatedTask);
		writeNum = sprintf(pBuff, "%s, %ld\n", g_appDefine[i].task.pcName, (UINT32)remainStackSize);
		pBuff += writeNum;
	}
}






