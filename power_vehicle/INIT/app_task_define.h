#ifndef __APP_TASK_DEFINE_H__
#define __APP_TASK_DEFINE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common_def.h"
#include "FreeRTOS.h"
#include "task.h"

#define TASK_STASK_SIZE_BYTES_64		64
#define TASK_STASK_SIZE_BYTES_128		128
#define TASK_STASK_SIZE_BYTES_256		256
#define TASK_STASK_SIZE_BYTES_512		512
#define TASK_STASK_SIZE_BYTES_1024		1024

#define TASK_PRIORITY_IDLE				(configMAX_PRIORITIES - 7)
#define TASK_PRIORITY_LOW				(configMAX_PRIORITIES - 6)
#define TASK_PRIORITY_BELOW_NORMAL		(configMAX_PRIORITIES - 5)
#define TASK_PRIORITY_NORMAL			(configMAX_PRIORITIES - 4)
#define TASK_PRIORITY_ABOVE_NORMAL		(configMAX_PRIORITIES - 3)
#define TASK_PRIORITY_HIGH				(configMAX_PRIORITIES - 2)
#define TASK_PRIORITY_REAL_TIME			(configMAX_PRIORITIES - 1)

#define DEVICE_ATTACH_APP_SIZE 10

struct task_define {
	TaskFunction_t pxTaskCode;
	char * pcName;
	UINT16 usStackDepth;
	void * pvParameters;
	UBaseType_t uxPriority;
	TaskHandle_t * pxCreatedTask;
};

struct queue_define {
	UINT32 uxQueueLength;
	UINT32 uxItemSize;
};

struct app_define {
	UINT8 tag;
	struct task_define task;
	struct queue_define queue;
};


struct device_attach_app_node {
	UINT8 tag;
	struct device_attach_app_node *next;
};

INT32 TaskInit(UINT8 tag, TaskFunction_t pfn, TaskHandle_t * const pTCB);
INT32 TaskCreate(void);
INT32 SendDataToApp(UINT8 isrFlag, UINT8 tag, struct platform_info* data);
void AttachDevices(UINT8 appTag, UINT8 devTag);
void DeattachDevices(UINT8 appTag, UINT8 devTag);
struct device_attach_app_node * GetDeviceAttachApp(UINT8 devTag);
void TaskRemainStack(void);


#ifdef __cplusplus
}
#endif

#endif
