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

static INT32 WheelSendData(UINT8 desTag, void *data, UINT16 dataLen)
{
	return SendDataToQueue(TAG_APP_WHEEL, desTag, data, dataLen);
}

static bool g_forceCalibrationFlag = false;
static void WheelAppForceProcess(struct platform_info *app)
{
	UINT16 force = *(UINT16 *)app->private_data;

	g_incPID.setpoint = g_wheelControl.force;
	IncPID(&g_incPID, force);
	g_neuralPID.setpoint = g_wheelControl.force;
	NeuralPID(&g_neuralPID, force);

	if (g_forceCalibrationFlag) {
		return;
	}

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

static void WheelAppRotateSpeedProcess(struct platform_info *app)
{
	struct rotate_speed *speed = (struct rotate_speed *)app->private_data;
	g_wheelControl.rpm = speed->wheelRpm;
	g_wheelControl.force = g_wheelControl.power / g_wheelControl.rpm;
}

static void WheelAppUiCalibration(void)
{
	struct platform_info *optDev = NULL;
	if (GetDeviceInfo(TAG_DEVICE_PWM_OUT, &optDev) != SUCC) {
		return;
	}
	
	g_forceCalibrationFlag = true;

	struct pwm_out pwmOut;
	pwmOut.dev = PWM_OUT_WHEEL;
	pwmOut.duty = 0;
	optDev->fops->write(optDev, &pwmOut, sizeof(pwmOut));

	UINT8 data = 0;
	WheelSendData(TAG_APP_FORCE, &data, sizeof(data));
}

static void WheelAppUiProcess(struct platform_info *app)
{
	struct ui_to_wheel uiData = *(struct ui_to_wheel *)app->private_data;
	
	switch (uiData.dataType) {
		case COMM_POWER_VEHICLE_SET_POWER:
		case KEY_RETURN_POWER_INC_DEC:
			g_wheelControl.power = uiData.data;		
			break;
		case KEY_KCOEF_INC_DEC:
			g_neuralPID.Kcoef = uiData.data / 100.0;
			break;
		case KEY_KP_INC_DEC:
			g_neuralPID.Kp = uiData.data / 100.0;
			break;
		case KEY_KI_INC_DEC:
			g_neuralPID.Ki = uiData.data / 100.0;
			break;
		case KEY_KD_INC_DEC:
			g_neuralPID.Kd = uiData.data / 100.0;
			break;
		case UI_CALIBRATION_FORCE:
			WheelAppUiCalibration();
			break;
		default:
			break;
	}

}

static void WheelProcess(struct platform_info *app)
{
	switch (app->tag) {
		case TAG_APP_FORCE:
			WheelAppForceProcess(app);
			break;
		case TAG_APP_ROTATE_SPEED:
			WheelAppRotateSpeedProcess(app);
			break;
		case TAG_APP_UI:
			WheelAppUiProcess(app);
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

static UINT8 g_wheelData = 0;
static struct platform_info g_appWheel = {
	.tag = TAG_APP_WHEEL,
	.private_data = (void *)&g_wheelData,
	.private_data_len = sizeof(g_wheelData),
};

static INT32 AppWheelInit(void)
{
	GetSysConfigOpt()->Read();
	if (GetSysConfigOpt()->sysConfig->Kcoef != 0xFFFF) {
		g_neuralPID.Kcoef = GetSysConfigOpt()->sysConfig->Kcoef / 100.0;
	}
	if (GetSysConfigOpt()->sysConfig->Kp != 0xFFFF) {
		g_neuralPID.Kp = GetSysConfigOpt()->sysConfig->Kp / 100.0;
	}
	if (GetSysConfigOpt()->sysConfig->Ki != 0xFFFF) {
		g_neuralPID.Ki = GetSysConfigOpt()->sysConfig->Ki / 100.0;
	}
	if (GetSysConfigOpt()->sysConfig->Kd != 0xFFFF) {
		g_neuralPID.Kd = GetSysConfigOpt()->sysConfig->Kd / 100.0;
	}

	static TaskHandle_t pwheelTCB = NULL;
	TaskInit(g_appWheel.tag, WheelTask, &pwheelTCB);
	RegisterApp(&g_appWheel);
	return SUCC;
}

module_init(AppWheelInit);



