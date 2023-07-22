#ifndef __ALG_H__
#define __ALG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common_def.h"

__packed struct fifo {
	UINT16 *data;
	UINT32 head;
	UINT32 tail;
};

__packed struct weight_moving_average_filter {
  struct fifo *fifo;
  UINT16 *weight;
  UINT16 fifoSize;
};

UINT16 WeightMovingAverageFilter(struct weight_moving_average_filter *weightMovingAverageFilter, UINT16 data);

#ifdef __cplusplus
}
#endif

#endif
