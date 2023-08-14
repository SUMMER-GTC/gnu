#ifndef __APP_MANAGER_H__
#define __APP_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common_def.h"
#include "app_rotate_speed.h"
#include "app_wheel.h"

#define APP_ATTATCH_DEVICE_SIZE 10
#define DEV_INTERVAL_BUFF_SIZE 10

#define DEV_INTERVAL_30MS 	10
#define DEV_INTERVAL_50MS 	50
#define DEV_INTERVAL_100MS 	100
#define DEV_INTERVAL_200MS 	200
#define DEV_INTERVAL_500MS 	500
#define DEV_INTERVAL_1000MS 1000

struct dev_interval {
	UINT8 tag;
	UINT16 interval;
};

struct app_data {
	UINT8 *data;
	UINT16 dataLen;
	UINT8 uiAppState;
};


void RegisterApp(struct platform_info *app);
INT32 GetAppInfo(UINT8 tag, struct platform_info **app);
INT32 SendDataToQueue(UINT8 srcTag, UINT8 desTag, void *data, UINT32 dataLen);

#ifdef __cplusplus
}
#endif

#endif


