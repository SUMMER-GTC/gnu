#include "device_manager.h"
#include "init.h"
#include "pwm_out.h"

static void GpioConfig(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;

		RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOB, ENABLE);

		GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_TIM12);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_TIM12);
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;    
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 
	
		GPIO_Init(GPIOB, &GPIO_InitStructure);	
}

static void PwmModeConfig(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12,ENABLE);
	
	// APB1 timer clock source TIMxCLK = 2 * PCLK1  
	//				PCLK1 = HCLK / 4 
	//				=> TIMxCLK = HCLK / 2 = SystemCoreClock /2
	// time frequency = TIMxCLK/(TIM_Prescaler+1) = (SystemCoreClock /2)/((SystemCoreClock/2)/1000000)*30 = 1000000/30 = 1/30MHz


	//APB2 timer clock source TIMxCLK = 2 * PCLK2  
	//				PCLK2 = HCLK / 2 
	//				=> TIMxCLK = HCLK  = SystemCoreClock 
	// time frequency = TIMxCLK/(TIM_Prescaler+1) = (SystemCoreClock )/((SystemCoreClock)/1000000)*30 = 1000000/30 = 1/30MHz

  TIM_TimeBaseStructure.TIM_Period = 1000-1; // 1KHz
	TIM_TimeBaseStructure.TIM_Prescaler = 84 - 1; // 84MHz / 84 = 1MHz
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM12, &TIM_TimeBaseStructure);

	/* PWM1 Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	    				
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	
  TIM_OCInitStructure.TIM_Pulse = 0;	  														
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;  	  
 

  TIM_OC1Init(TIM12, &TIM_OCInitStructure);	 							
	TIM_OC1PreloadConfig(TIM12, TIM_OCPreload_Enable);

  TIM_OC2Init(TIM12, &TIM_OCInitStructure);	 							
	TIM_OC2PreloadConfig(TIM12, TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(TIM12, ENABLE);
	TIM_Cmd(TIM12, ENABLE);		
}

static INT32 DeviceInit(void *dev)
{
	struct platform_info *pDev = (struct platform_info *)dev;
	pDev->states |= DEVICE_INIT;

	GpioConfig();
	PwmModeConfig();
	
	return SUCC;
}

static INT32 DeviceRead(void *dev)
{
	UNUSED(dev);

	PrintfLogInfo(DEBUG_LEVEL, "[device_led][DeviceRead]\n");

	return SUCC;
}

static INT32 DeviceWrite(void *dev, void *data, UINT32 dataLen)
{
	UNUSED(dev);
	UNUSED(dataLen);
	UINT32 wheelDuty = *(UINT32*)data;
	TIM_SetCompare1(TIM12, wheelDuty);
	TIM_SetCompare2(TIM12, wheelDuty);

	return SUCC;
}

static struct file_operations g_fops = {
	.read = DeviceRead,
	.write = DeviceWrite,
};

static void DeviceIntervalCall (void *dev)
{
	DeviceSampleData(SEND_FROM_ISR, TAG_APP_WHEEL, dev);
}

static struct platform_info g_devicePwmOut = {
	.tag = TAG_DEVICE_PWM_OUT,
	.fops = &g_fops,
	.setInterval = 30,
	.IntervalCall = DeviceIntervalCall
};

static INT32 DevicePwmOutInit(void)
{
	if (DeviceInit((void*)&g_devicePwmOut) != SUCC) {
		return FAIL;
	}

	RegisterDevice(&g_devicePwmOut);
	return SUCC;
}

module_init(DevicePwmOutInit);











