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
  UINT8 dataType;
  UINT16 rpm;
  UINT16 power;
  UINT16 spo2;
  UINT16 vo2;
  UINT16 vco2;
  UINT16 heartRate;
  UINT16 lbp;
  UINT16 hbp;
} __packed;

#define UI_DISPLAY_DATA_OFFSET(member) (UINT32)&(((struct ui_display_data *)0)->member)

#define UI_CALIBRATION_FORCE 0x11

#define UI_DEFAULT_DATA_KCOEF (12)
#define UI_DEFAULT_DATA_KP    (40)
#define UI_DEFAULT_DATA_KI    (35)
#define UI_DEFAULT_DATA_KD    (40)

#ifdef __cplusplus
}
#endif

#endif
