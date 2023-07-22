#include "alg.h"

UINT16 WeightMovingAverageFilter(struct weight_moving_average_filter *weightMovingAverageFilter, UINT16 data)
{
	UINT32 retVal = 0;
	UINT32 head = 0;
	UINT16 weightSum = 0;
	UINT32 valSum = 0;

	// fifo full
	if ((weightMovingAverageFilter->fifo->tail + 1) % weightMovingAverageFilter->fifoSize == weightMovingAverageFilter->fifo->head) { 

		head = weightMovingAverageFilter->fifo->head;

		for (UINT32 i = 0; i < (weightMovingAverageFilter->fifoSize - 1); i++) {
			valSum += weightMovingAverageFilter->fifo->data[head] * weightMovingAverageFilter->weight[i];
			weightSum += weightMovingAverageFilter->weight[i];
			head = (head + 1) % weightMovingAverageFilter->fifoSize;
		}
		retVal = valSum / weightSum;
		weightMovingAverageFilter->fifo->head = (weightMovingAverageFilter->fifo->head + 1) % weightMovingAverageFilter->fifoSize;
	}

	weightMovingAverageFilter->fifo->data[weightMovingAverageFilter->fifo->tail] = data;
	weightMovingAverageFilter->fifo->tail = (weightMovingAverageFilter->fifo->tail + 1) % weightMovingAverageFilter->fifoSize;

	return retVal;
}

