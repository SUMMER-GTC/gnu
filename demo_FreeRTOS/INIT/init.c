#include "init.h"
#include "stdio.h"
#include "device_manager.h"
#include "app_manager.h"

extern UINT32 initcall$$Base;
extern UINT32 initcall$$Limit;

static UINT32 *__start_code = &initcall$$Base;
static UINT32 *__end_code = &initcall$$Limit;

static INT32 DoInit(initcall_t fun)
{
	if (fun == NULL) {
		return FAIL;
	}
	fun();
	return SUCC;
}

INT32 Init(void)
{
	initcall_t callFunc = NULL;

	for (UINT32 *funAddr = __start_code; funAddr < __end_code; funAddr++) {
		callFunc = (initcall_t)(*(funAddr));
		DoInit(callFunc);	
	}

	DeviceManagerInit();
	AppManagerInit();

	return SUCC;
}

#if 0
#include "init.h"
#include "stdio.h"
#include "app_task_define.h"


void SystemConfigInit(void)
{
	taskENTER_CRITICAL();
	
	Init();
	TaskCreate();
	
	vTaskDelete(defaultTaskHandle);
	taskEXIT_CRITICAL();
}

int fputc(int c, FILE * f)
{
	HAL_UART_Transmit(&huart3, (uint8_t*)&c, 1, 1000);
	return c;
}
#endif



