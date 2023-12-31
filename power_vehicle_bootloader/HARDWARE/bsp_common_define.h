#ifndef __BSP_COMMON_H__
#define	__BSP_COMMON_H__

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f4xx.h" 

#define SUCC 0
#define FAIL -1
  
typedef void (*jump_t)(void); 

#define DEVICE_NAME_SIZE 32
#define HARDWARE_VER_SIZE 32

struct calibration {
	u16 calibratedFlag;
	vs16 value;
};

struct sys_config {
	u32 firstUseId;
	u32 otaState;
	u32 hardWareErrCnt;
	u32 deviceNameErrCnt;
	u32 romCheckErrCnt;
	u16 Kcoef;
	u16 Kp;
	u16 Ki;
	u16 Kd;
	struct calibration cal;
	u32 languageVer; // 0 is english, 1 is chinese
} __packed;

struct sys_config_opt {
	struct sys_config *sysConfig;
	void (*Init)(void);
	void (*Read)(void);
	void (*Write)(void);
} __packed;

#ifdef __cplusplus
}
#endif

#endif /* __USART1_H */
