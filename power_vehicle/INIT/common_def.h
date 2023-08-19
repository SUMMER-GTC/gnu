#ifndef __COMMON_DEF_H__
#define __COMMON_DEF_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "string.h"

#ifndef __weak
#define __weak  __attribute__((weak))
#endif

#define UINT8 	uint8_t
#define INT8  	int8_t
#define UINT16 	uint16_t
#define INT16 	int16_t
#define UINT32 	uint32_t
#define INT32 	int32_t
#define UINT64 	uint64_t
#define INT64 	int64_t

#define UNUSED(X) (void)X      /* To avoid gcc/g++ warnings */

#ifndef SUCC
#define SUCC (0UL)
#endif
  
#ifndef FAIL
#define FAIL (-1UL)
#endif

#define SEND_FROM_ISR 		1
#define SEND_FROM_NORMAL 	0

#define DEV_ATTACH_APP_LIST_FLAG 	1
#define PLATFOR_INFO_LIST_FLAG		2

#define PRINT_DEBUG_INFO
// #define PRINT_WARNING_INFO
// #define PRINT_ERROR_INFO

#define DEBUG_LEVEL		0
#define WARNING_LEVEL	1
#define ERROR_LEVEL		2

#define DEBUG_INFO_STR		"[DEBUG]"
#define WARNING_INFO_STR 	"[WARNING]"
#define ERROR_INFO_STR		"[ERROR]"

#define PRINTF_BUFF_SIZE 128
#define COMMON_TICK_STR_SIZE 16

enum app_device_tag {
	TAG_START				= 0,
		
	TAG_APP_START			= TAG_START,
	TAG_APP_UI 				= TAG_APP_START,
	TAG_APP_COMPUTER 	= 1,
	TAG_APP_WINK,
	TAG_APP_WHEEL,
	TAG_APP_ROTATE_SPEED,
	TAG_APP_FORCE,
	TAG_APP_DATA_STORAGE,
	TAG_APP_END,

	TAG_DEVICE_START		= TAG_APP_END,
	TAG_DEVICE_UART_SCREEN  = TAG_DEVICE_START,
	TAG_DEVICE_LED,
	TAG_DEVICE_PWM_OUT,
	TAG_DEVICE_PWM_CAPTURE,
	TAG_DEVICE_TASEOMETER,
	TAG_DEVICE_UPPER_COMPUTER,
	TAG_DEVICE_MAX6950,
	TAG_DEVICE_END,
	
	TAG_END 				= TAG_DEVICE_END
};

struct file_operations {
	INT32 (*open) (void *);
	INT32 (*close) (void *);
	INT32 (*read) (void *);
	INT32 (*write) (void *, void *data, UINT32 dataLen);
	INT32 (*ioctl) (void *, UINT32 cmd, void *data, UINT32 dataLen);
};

struct platform_info {
	UINT8 tag;
	struct file_operations *fops;
	void *private_data;
	UINT16 private_data_len;
	UINT16 setInterval;
	void (*IntervalCall) (void *);
	UINT8 states;
};

struct platform_info_node {
	struct platform_info *info;
	struct platform_info_node *next;
};

struct round_robin_queue {
	UINT16 front;
	UINT16 tail;
	UINT8  *data;
};

struct common_time {
	UINT8 hour;
	UINT8 minute;
	UINT8 second;
	UINT16 msec;
} __packed;

#define DEVICE_NAME_SIZE 32
#define HARDWARE_VER_SIZE 32

struct calibration {
	bool calibratedFlag;
	INT16 value;
};

struct sys_config {
	UINT32 firstUseId;
	UINT32 otaState;
	UINT32 hardWareErrCnt;
	UINT32 deviceNameErrCnt;
	UINT32 romCheckErrCnt;
	UINT16 Kcoef;
	UINT16 Kp;
	UINT16 Ki;
	UINT16 Kd;
	struct calibration cal;
} __packed;

struct sys_config_opt {
	struct sys_config *sysConfig;
	void (*Init)(void);
	void (*Read)(void);
	void (*Write)(void);
} __packed;

struct platform_info_node *CommonInitList(void);
INT32 CommonInsertNodeToListTail(struct platform_info_node *headNode, struct platform_info *info);
INT32 CommonDeleteNodeFromList(struct platform_info_node *headNode, struct platform_info *info);
struct platform_info *GetPlatformInfo(UINT8 tag);
void PrintfLogInfo(UINT8 level, char *str, ...);
void CommonTime(char *timeStr);

#ifdef __cplusplus
}
#endif

#endif


