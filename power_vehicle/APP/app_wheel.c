#include "app_wheel.h"

#include "app_manager.h"
#include "app_task_define.h"
#include "init.h"
#include "stdio.h"
#include "queue.h"		
#include "alg.h"
#include "device_manager.h"

struct wheel_control {
	UINT16 force;
	UINT16 rpm;
	UINT16 power;
} __packed;

static struct wheel_control g_wheelControl = { 
	.force = 0, 
	.rpm = 0, 
	.power = 10
};

static struct inc_pid g_incPID = {
	.setpoint = 0,
	.output = 0,
  .Kp = 1,
  .Ki = 1, 
  .Kd = 1, 
  .error1 = 0,
  .error2 = 0,
  .error = 0
};

static struct neural_pid g_neuralPID = {
  .setpoint = 5000,         
  .Kcoef = 0.12,            
  .Kp = 0.4,               
  .Ki = 0.35,               
  .Kd = 0.4,               
  .error1 = 0.0,           
  .error2 = 0.0,           
  .deadband = (10000 - (- 10000)) * 0.0005,         
  .output = 0.0,           
  .outputPercentage = 0.0, 
  .maximum = 10000,          
  .minimum = -10000,          
  .Wp = 0.1,               
  .Wi = 0.1,               
  .Wd = 0.1,               
  .learningMode = HEBBE_LEARNING_MODE,
};

static void WheelAppForceProcess(struct platform_info *dev)
{
	UINT16 force = *(UINT16 *)dev->private_data;

	g_incPID.setpoint = g_wheelControl.force;
	IncPID(&g_incPID, force);
	g_neuralPID.setpoint = g_wheelControl.force;
	NeuralPID(&g_neuralPID, force);

	struct platform_info *optDev = NULL;
	if (GetDeviceInfo(TAG_DEVICE_PWM_OUT, &optDev) != SUCC) {
		return;
	}
	
	struct pwm_out pwmOut;
	pwmOut.dev = PWM_OUT_WHEEL;
	pwmOut.duty = 750;
	optDev->fops->write(optDev, &pwmOut, sizeof(pwmOut));

	pwmOut.dev = PWM_OUT_EE_SY110;
	pwmOut.duty = 500;
	optDev->fops->write(optDev, &pwmOut, sizeof(pwmOut));
}

static void WheelAppRotateSpeedProcess(struct platform_info *dev)
{
	g_wheelControl.rpm = *(UINT16 *)dev->private_data;
	g_wheelControl.force = g_wheelControl.power / g_wheelControl.rpm;
}

static void WheelAppComputerProcess(struct platform_info *dev)
{
	g_wheelControl.power = *(UINT8 *)dev->private_data;
}

static void WheelAppUiProcess(struct platform_info *dev)
{
	g_wheelControl.power = *(UINT8 *)dev->private_data;
}

static void WheelProcess(struct platform_info *dev)
{
	switch (dev->tag) {
		case TAG_APP_FORCE:
			WheelAppForceProcess(dev);
			break;
		case TAG_APP_ROTATE_SPEED:
			WheelAppRotateSpeedProcess(dev);
			break;
		case TAG_APP_COMPUTER:
			WheelAppComputerProcess(dev);
			break;
		case TAG_APP_UI:
			WheelAppUiProcess(dev);
			break;
		default:
			break;
	}
}

void WheelTask(void *pvParameters)
{
	struct platform_info queueData;
	QueueHandle_t xQueue = (QueueHandle_t)pvParameters;
	for(;;) {
		xQueueReceive(xQueue, &queueData, portMAX_DELAY);
		
		WheelProcess(&queueData);
	}

}

static struct platform_info g_appWheel = {
	.tag = TAG_APP_WHEEL,
};

static INT32 AppWheelInit(void)
{
	static TaskHandle_t pwheelTCB = NULL;
	TaskInit(g_appWheel.tag, WheelTask, &pwheelTCB);
	RegisterApp(&g_appWheel);
	return SUCC;
}

module_init(AppWheelInit);



