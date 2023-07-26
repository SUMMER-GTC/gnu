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

void IncPID(struct inc_pid *pid, float curVal)
{
	float u = pid->output;
	pid->error = pid->setpoint - curVal;

	float deltaU = pid->Kp * (pid->error - pid->error1) \
								+ pid->Ki * pid->error \
								+ pid->Kd * (pid->error - 2 * pid->error1 + pid->error2);

	pid->error2 = pid->error1;
	pid->error1 = pid->error;

	pid->output = u + deltaU;
}

void NeuralPID(struct neural_pid *pid, float curVal)
{
	float x[3] = { 0.0 };
	float w[3] = { 0.0 };
	float deltaU = 0.0;

	float error = pid->setpoint - curVal;
	float u = pid->output;

	if (fabs(error) > pid->deadband) {
		x[0] = error - pid->error1;
		x[1] = error;
		x[2] = error - 2 * pid->error1 + pid->error2;

		float normalization = fabs(pid->Wp) + fabs(pid->Wi) + fabs(pid->Wd);
		w[0] = pid->Wp / normalization;
		w[1] = pid->Wi / normalization;
		w[2] = pid->Wd / normalization;

		deltaU = pid->Kcoef * (x[0] * w[0] + x[1] * w[1] + x[2] * w[2]);
	} else {
		deltaU = 0;
	}

	u = u + deltaU;
	if (u > pid->maximum) {
		u = pid->maximum;
	} else if (u < pid->minimum) {
		u = pid->minimum;
	}

	pid->output = u;
	pid->outputPercentage = (pid->output - pid->minimum) * 100 / (pid->maximum - pid->minimum);

	if (pid->learningMode == IMPROVED_HEBBE_LEARNING_MODE) {
		pid->Wp = pid->Wp + pid->Kp * error * u * (error - pid->error1);
		pid->Wi = pid->Wi + pid->Ki * error * u * (error - pid->error1);
		pid->Wd = pid->Wd + pid->Kd * error * u * (error - pid->error1);
	} else {
		pid->Wp = pid->Wp + pid->Kp * error * u * x[0];
		pid->Wi = pid->Wi + pid->Ki * error * u * x[1];
		pid->Wd = pid->Wd + pid->Kd * error * u * x[2];
	}

	pid->error2 = pid->error1;
	pid->error1 = error;
}

