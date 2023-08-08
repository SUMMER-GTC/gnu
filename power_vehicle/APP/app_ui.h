#ifndef __APP_UI_H__
#define __APP_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common_def.h"
#include "FreeRTOS.h"
#include "task.h"

#define UI_DATA_BUFF_SIZE 10

struct ui_display_data {
  UINT16 rpm;
  UINT16 power;
  UINT16 spo2;
  UINT16 vo2;
  UINT16 vco2;
  UINT16 heartRate;
  UINT16 lbp;
  UINT16 hbp;
} __packed;

#ifdef __cplusplus
}
#endif

#endif
