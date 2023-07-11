#include "init.h"
#include "stdio.h"
#include "device_manager.h"
#include "app_manager.h"

extern UINT32 __initcall_start;
extern UINT32 __initcall_end;

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
	volatile initcall_t callFunc = NULL;
	volatile UINT32 *funAddr = &__initcall_start;

	do {
		callFunc = (initcall_t)(*(funAddr));
		DoInit(callFunc);	
	} while(++funAddr < &__initcall_end);

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



