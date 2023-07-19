#ifndef __DEVICE_MANAGER_H__
#define __DEVICE_MANAGER_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "common_def.h"

enum device_states {
	DEVICE_INIT 				= (1 << 0),
 	DEVICE_OPEN					= (1 << 1),
	DEVICE_SET_INTERVAL	= (1 << 2),
};

void RegisterDevice(struct platform_info *dev);
INT32 GetDeviceInfo(UINT8 tag, struct platform_info **dev);
INT32 DeviceCreateSetIntervalTimer(void);
INT32 DeviceSampleData(UINT8 isrFlag, UINT8 tag, struct platform_info *data);

#ifdef __cplusplus
}
#endif

#endif
