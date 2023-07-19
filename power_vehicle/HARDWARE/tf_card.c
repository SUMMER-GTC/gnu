#include "device_manager.h"
#include "init.h"
#include "tf_card.h"

static INT32 DeviceInit(void *dev)
{
	struct platform_info *pDev = (struct platform_info *)dev;
	pDev->states |= DEVICE_INIT;
	

	return SUCC;
}

static INT32 DeviceRead(void *dev)
{
	UNUSED(dev);
	
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
	DeviceSampleData(SEND_FROM_ISR, TAG_APP_FORCE, dev);
}

static struct platform_info g_deviceTFCard = {
	.tag = TAG_DEVICE_TASEOMETER,
	.fops = &g_fops,
	.setInterval = 30,
	.IntervalCall = DeviceIntervalCall
};

static INT32 DeviceTFCardInit(void)
{
	if (DeviceInit((void*)&g_deviceTFCard) != SUCC) {
		return FAIL;
	}

	RegisterDevice(&g_deviceTFCard);
	return SUCC;
}

module_init(DeviceTFCardInit);











