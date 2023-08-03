#ifndef __PWM_OUT_H__
#define __PWM_OUT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "sys.h"

typedef enum {
  PWM_OUT_WHEEL = 0,
  PWM_OUT_EE_SY110 = 1,
} PWM_OUT_DEVICE;

struct pwm_out {
  UINT8 dev;
  UINT32 duty;
} __packed;

#ifdef __cplusplus
}
#endif

#endif
