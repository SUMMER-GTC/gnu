#ifndef __APP_ROTATE_SPEED_H__
#define __APP_ROTATE_SPEED_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common_def.h"
#include "FreeRTOS.h"
#include "task.h"

struct rotate_speed {
  UINT16 wheelRpm;
  UINT16 displayRpm;
} __packed;

#ifdef __cplusplus
}
#endif

#endif
