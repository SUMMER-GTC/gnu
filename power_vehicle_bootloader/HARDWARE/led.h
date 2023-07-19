#ifndef __LED_H__
#define __LED_H__	 

#ifdef __cplusplus
extern "C" {
#endif

#include "sys.h"

#define LED0 PFout(9) 
#define LED1 PFout(10) 

void LED_Init(void); 

#ifdef __cplusplus
}
#endif

#endif
