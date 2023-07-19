#include "device_manager.h"
#include "stm32f4xx_conf.h"
#include "init.h"
#include "led.h"

static INT32 DeviceInit(void *dev)
{
	struct platform_info *pDev = (struct platform_info *)dev;
	pDev->states |= DEVICE_INIT;

    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);	 

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 		     
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    GPIO_SetBits(GPIOF, GPIO_Pin_9);
	
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
    if (dataLen > 1) {
        return FAIL;
    }

    BitAction bitVal = *(BitAction *)data;
    GPIO_WriteBit(GPIOF, GPIO_Pin_9, bitVal);

	return SUCC;
}

static struct file_operations g_fops = {
	.read = DeviceRead,
	.write = DeviceWrite,
};

static void LedIntervalCall(void *dev)
{
	DeviceSampleData(SEND_FROM_ISR, TAG_APP_WINK, dev);
}

static struct platform_info g_deviceLed = {
	.tag = TAG_DEVICE_LED,
	.fops = &g_fops,
	.setInterval = 200,
	.IntervalCall = LedIntervalCall
};

static INT32 DeviceLedInit(void)
{
	if (DeviceInit((void*)&g_deviceLed) != SUCC) {
		return FAIL;
	}

	RegisterDevice(&g_deviceLed);
	return SUCC;
}

module_init(DeviceLedInit);