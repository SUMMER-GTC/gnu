#include "device_manager.h"
#include "init.h"
#include "taseometer.h"

#define ADC_RESOLUTION_12BIT 	(0x0001 << 12)
#define ADC_REF_VOLTAGE 			(3.3)
#define ADC_MILLIVOLT_FACTOR	(1000)

static UINT16 g_adcConvertedValue = 0;
static UINT16 g_millivolt = 0;

static void GpioConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; 
	GPIO_Init(GPIOC, &GPIO_InitStructure);		
}

static void ADCModeConfig(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
	
  // ------------------DMA Init--------------------------
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE); 
	DMA_InitStructure.DMA_PeripheralBaseAddr = ((UINT32)&ADC1->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (UINT32)&g_adcConvertedValue;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = 1;	
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable; 
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; 
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;  
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;  
  DMA_InitStructure.DMA_Channel = DMA_Channel_0; 
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);
  DMA_Cmd(DMA2_Stream0, ENABLE);
	

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 , ENABLE);
  // -------------------ADC Common------------------------
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;  
  ADC_CommonInit(&ADC_CommonInitStructure);
	
  // -------------------ADC Init--------------------------
	ADC_StructInit(&ADC_InitStructure);
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE; 
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; 
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 1;                                    
  ADC_Init(ADC1, &ADC_InitStructure);
  //---------------------------------------------------------------------------
	
  ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_56Cycles);

  // DMA request after last transfer (Single-ADC mode)
  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
  ADC_DMACmd(ADC1, ENABLE);
	
  ADC_Cmd(ADC1, ENABLE);  
  ADC_SoftwareStartConv(ADC1);
}

static INT32 DeviceInit(void *dev)
{
	struct platform_info *pDev = (struct platform_info *)dev;
	pDev->states |= DEVICE_INIT;
	
	GpioConfig();
	ADCModeConfig();

	return SUCC;
}

static INT32 DeviceRead(void *dev)
{
	struct platform_info *pDev = (struct platform_info *)dev;
	float adcVoltage = 0;
	UINT16 *pmillivolt = pDev->private_data;

	adcVoltage = (float)g_adcConvertedValue / ADC_RESOLUTION_12BIT * ADC_REF_VOLTAGE;
	*pmillivolt = (UINT16)(adcVoltage * ADC_MILLIVOLT_FACTOR);
	
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
	DeviceSampleData(SEND_FROM_NORMAL, TAG_APP_FORCE, dev);
}

static struct platform_info g_deviceTaseometer = {
	.tag = TAG_DEVICE_TASEOMETER,
	.fops = &g_fops,
	.setInterval = 30,
	.IntervalCall = DeviceIntervalCall,
	.private_data = &g_millivolt,
	.private_data_len = sizeof(g_millivolt),
};

static INT32 DeviceTaseometerInit(void)
{
	if (DeviceInit((void*)&g_deviceTaseometer) != SUCC) {
		return FAIL;
	}

	RegisterDevice(&g_deviceTaseometer);
	return SUCC;
}

module_init(DeviceTaseometerInit);











