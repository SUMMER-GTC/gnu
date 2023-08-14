#ifndef __APP_WHEEL_H__
#define __APP_WHEEL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common_def.h"
#include "FreeRTOS.h"
#include "task.h"


struct ui_to_wheel {
  UINT8 dataType;
  UINT16 data;
} __packed;

#ifdef __cplusplus
}
#endif

#endif
