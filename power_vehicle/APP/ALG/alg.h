#ifndef __ALG_H__
#define __ALG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "math.h"
#include "common_def.h"

typedef enum {
  HEBBE_LEARNING_MODE = 0,
  IMPROVED_HEBBE_LEARNING_MODE = 1,
} NEURAL_PID_LEARNIG_MODE;

__packed struct neural_pid {
  float setpoint;         // set value
  float Kcoef;            // neural output coefficient
  float Kp;               // proportional learning rate
  float Ki;               // integer learning rate
  float Kd;               // derivative learning rate    
  float error1;           // last error
  float error2;           // last two times error
  float deadband;         // deadband
  float output;           // output
  float outputPercentage; // output percentage
  float maximum;          // output upper limit
  float minimum;          // output lower limit
  float Wp;               // proportional weight coefficient
  float Wi;               // integer weight coefficient
  float Wd;               // derivative weight coefficient
  int learningMode;       // learning mode, 0: hebbe learning mode, 1: improved hebbe learning mode
};

__packed struct inc_pid {
  float setpoint; // set value
  float output;   // u
  float Kp;       // proportional constant
  float Ki;       // integral constant
  float Kd;       // derivative constant
  float error1;   // error[n-1]
  float error2;   // error[n-2]
  float error;    // error[n]
};
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
void IncPID(struct inc_pid *pid, float curVal);
void NeuralPID(struct neural_pid *pid, float curVal);

#ifdef __cplusplus
}
#endif

#endif
