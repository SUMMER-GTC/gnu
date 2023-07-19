#ifndef __INIT_H__
#define __INIT_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "common_def.h"

typedef INT32 (*initcall_t)(void);

#define __used			__attribute__((__used__))
#define __attribute_used__	__used				/* deprecated */


#define __define_initcall(fn) \
		static initcall_t __initcall_##fn __attribute_used__ \
		__attribute__((__section__(".initcall"))) = fn

#define module_init(x) __define_initcall(x);

INT32 Init(void);

#ifdef __cplusplus
}
#endif

#endif

