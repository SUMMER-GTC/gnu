#include "device_manager.h"
#include "init.h"
#include "pwm_capture.h"

#define PWM_CAPTURE_TIM_PERIOD 10000
#define SECONDE_PER_MINUTE 60
#define ROTATE_TIME_OUT 30

enum {
	PWM_CAPTURE_IDLE = 0,
	PWM_CAPTURE_FIRST_RISE_EDGE,
	PWM_CAPTURE_FIRST_FALL_EDGE,
	PWM_CAPTURE_SECOND_RISE_EDGE,
};

__packed struct pwm_capture {
	UINT32 period;
	UINT32 duty;
	UINT32 periodUpdate;
};

static struct pwm_capture g_pwmCapture = { 0 };
static UINT16 g_rpm = 0;

static void FrequencyDetect(UINT8 pinLevel)
{
	static UINT8 lastPinLevel = 1;
	static UINT8 captureState = PWM_CAPTURE_IDLE;

	switch(captureState) {
		case PWM_CAPTURE_IDLE:
			if (lastPinLevel == 0 && pinLevel == 1) { 
				captureState = PWM_CAPTURE_FIRST_RISE_EDGE;
				g_pwmCapture.periodUpdate = 0;
				TIM_SetCounter(TIM2, 0);
			}
		break;
		case PWM_CAPTURE_FIRST_RISE_EDGE:
			if (lastPinLevel == 1 && pinLevel == 0) { 
				captureState = PWM_CAPTURE_FIRST_FALL_EDGE;
				g_pwmCapture.duty = TIM_GetCounter(TIM2);
			} else {
				captureState = PWM_CAPTURE_IDLE;
			}
		break;
		case PWM_CAPTURE_FIRST_FALL_EDGE:
			if (lastPinLevel == 0 && pinLevel == 1) { 
				captureState = PWM_CAPTURE_FIRST_RISE_EDGE;
				if (g_pwmCapture.duty < 2) { // 0.2ms, pulse width less than 2 ms

				}
				g_pwmCapture.period = TIM_GetCounter(TIM2) + g_pwmCapture.periodUpdate;
				g_pwmCapture.periodUpdate = 0;
				TIM_SetCounter(TIM2, 0);
			} else {
				captureState = PWM_CAPTURE_IDLE;
			}
		break;
		default:
			captureState = PWM_CAPTURE_IDLE;
		break;
	}
	lastPinLevel = pinLevel;
}

static UINT32 g_pulseCnt = 0;

void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line13) != RESET) {
		FrequencyDetect(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13));
		g_pulseCnt++;
		EXTI_ClearITPendingBit(EXTI_Line13);
	}
}

static void GpioConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOB, ENABLE);  // B13
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	EXTI_InitTypeDef EXTI_InitStructure;
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource13);
  EXTI_InitStructure.EXTI_Line = EXTI_Line13;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
}

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
		g_pwmCapture.periodUpdate += PWM_CAPTURE_TIM_PERIOD;
		TIM_ClearITPendingBit(TIM2 , TIM_IT_Update);
	}
}

static void CaptureTimerConfig(void)
{
	NVIC_InitTypeDef NVIC_InitStructure; 

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;

	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	// timer source TIMxCLK = 2 * PCLK1  
  //				PCLK1 = HCLK / 4 
  //				=> TIMxCLK=HCLK/2=SystemCoreClock/2=84MHz
	// set timer frequency=TIMxCLK/(TIM_Prescaler+1)=10000Hz
	// timer interrupt = 10000 * (1 / 10000) = 1Hz = 1s
	TIM_TimeBaseStructure.TIM_Period = PWM_CAPTURE_TIM_PERIOD - 1;
  TIM_TimeBaseStructure.TIM_Prescaler = 8400-1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	// normal timer TIMx, x[2,3,4,5]
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);	
}

static INT32 DeviceInit(void *dev)
{
	struct platform_info *pDev = (struct platform_info *)dev;
	pDev->states |= DEVICE_INIT;

	GpioConfig();
	CaptureTimerConfig();
	
	return SUCC;
}

static INT32 DeviceRead(void *dev)
{
	struct platform_info *pDev = (struct platform_info *)dev;
	if (!(pDev->states & DEVICE_INIT)) {
		return FAIL;
	}

	double revolutionsPerSecond = 0;
	UINT16 *prpm = pDev->private_data;

	if (g_pwmCapture.period == 0) {
		revolutionsPerSecond = 0;
	} else {
		revolutionsPerSecond = 1.0 / ((double)g_pwmCapture.period / PWM_CAPTURE_TIM_PERIOD);		
	}

	if (g_pwmCapture.periodUpdate > ROTATE_TIME_OUT) {
		revolutionsPerSecond = 0;
	}

	*prpm = (UINT32)(revolutionsPerSecond * SECONDE_PER_MINUTE);

	return SUCC;
}

static INT32 DeviceWrite(void *dev, void *data, UINT32 dataLen)
{
	UNUSED(dev);
	UNUSED(data);
	UNUSED(dataLen);

	return SUCC;
}

static struct file_operations g_fops = {
	.read = DeviceRead,
	.write = DeviceWrite,
};

static void DeviceIntervalCall (void *dev)
{
	DeviceSampleData(SEND_FROM_NORMAL, TAG_APP_ROTATE_SPEED, dev);
	DeviceSampleData(SEND_FROM_NORMAL, TAG_APP_DATA_STORAGE, dev);
}

static struct platform_info g_devicePwmCapture = {
	.tag = TAG_DEVICE_PWM_CAPTURE,
	.fops = &g_fops,
	.setInterval = 30,
	.IntervalCall = DeviceIntervalCall,
	.private_data = &g_rpm,
	.private_data_len = sizeof(g_rpm),
};

static INT32 DevicePwmCaptureInit(void)
{
	if (DeviceInit((void*)&g_devicePwmCapture) != SUCC) {
		return FAIL;
	}

	RegisterDevice(&g_devicePwmCapture);
	return SUCC;
}

module_init(DevicePwmCaptureInit);











