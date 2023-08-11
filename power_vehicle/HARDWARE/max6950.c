#include "device_manager.h"
#include "stm32f4xx_conf.h"
#include "init.h"
#include "max6950.h"

static struct max6959_config g_max6950Conf[] = {
  { MAX6950_REG_CONFIGURATION,  MAX6950_SHUTDOWN_ENABLE },
  { MAX6950_REG_DECODE,         MAX6950_ALL_DECODE }, 
  { MAX6950_REG_INTENSITY,      MAX6950_INTENSITY_16_16 },
  { MAX6950_REG_SCAN_LIMIT,     MAX6950_SCAN_LIMIT_2 },
  { MAX6950_REG_DIGIT_0,        MAX6950_DIGIT_0 },
  { MAX6950_REG_DIGIT_1,        MAX6950_DIGIT_0 },
  { MAX6950_REG_DIGIT_2,        MAX6950_DIGIT_0 },
  { MAX6950_REG_CONFIGURATION,  MAX6950_SHUTDOWN_DISABLE }, 
};

static void SPI_CS_High(void)
{
  GPIO_SetBits(GPIOA, GPIO_Pin_4);
}

static void SPI_CS_Low(void)
{
  GPIO_ResetBits(GPIOA, GPIO_Pin_4);
}

static void SpiInit(void)
{
  SPI_InitTypeDef  SPI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  // PA4-->SPI1_CS  PA5-->SPI1_SCK  PA6-->SPI1_MISO  PA7-->SPI1_MOSI  
  /* 使能 MAX6950_SPI 及GPIO 时钟 */
  /*!< SPI_FLASH_SPI_CS_GPIO, SPI_FLASH_SPI_MOSI_GPIO, 
       SPI_FLASH_SPI_MISO_GPIO,SPI_FLASH_SPI_SCK_GPIO 时钟使能 */
  RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOA, ENABLE);

  /*!< SPI_FLASH_SPI 时钟使能 */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
 
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);   // SPI1_CLK
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);   // SPI1_MOSI
  
  /*!< 配置 SPI_FLASH_SPI 引脚: SCK */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
	/*!< 配置 SPI_FLASH_SPI 引脚: MOSI */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  

	/*!< 配置 SPI_FLASH_SPI 引脚: CS */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* 停止信号 FLASH: CS引脚高电平*/
  SPI_CS_High();

  /* SPI1 AT APB2, CLK IS 84MHz */
  SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64; // 1.3MHz
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_Init(SPI1, &SPI_InitStructure);

  SPI_Cmd(SPI1, ENABLE);
}

static void Max6950_SPI_Send(UINT8 addr, UINT8 data)
{
  UINT16 sendData = (addr << 8) | data;

  SPI_CS_Low();
  UINT32 spiTimeOut = SPIT_FLAG_TIMEOUT;
  while (SPI_I2S_GetFlagStatus(MAX6950_SPI, SPI_I2S_FLAG_TXE) == RESET)
  {
    if((spiTimeOut--) == 0) return;
  }

  SPI_I2S_SendData(MAX6950_SPI, sendData);

  spiTimeOut = SPIT_FLAG_TIMEOUT;
  while (SPI_I2S_GetFlagStatus(MAX6950_SPI, SPI_I2S_FLAG_BSY) == SET)
  {
    if((spiTimeOut--) == 0) return;
  }
  SPI_CS_High();
}

static INT32 DeviceInit(void)
{
	SpiInit();

  for (UINT8 i = 0; i < sizeof(g_max6950Conf) / sizeof(struct max6959_config); i++) {
    Max6950_SPI_Send(g_max6950Conf[i].reg, g_max6950Conf[i].data);
  }

	return SUCC;
}

static INT32 DeviceWrite(void *dev, void *data, UINT32 dataLen)
{
  UINT16 rpm = *(UINT16 *)data;
  UINT8 uint = rpm % 10;
  UINT8 decade = rpm / 10 % 10;
  UINT8 hundred = rpm / 100;

  Max6950_SPI_Send(MAX6950_REG_DIGIT_0, hundred);
  Max6950_SPI_Send(MAX6950_REG_DIGIT_1, decade);
  Max6950_SPI_Send(MAX6950_REG_DIGIT_2, uint);

	return SUCC;
}

static struct file_operations g_fops = {
	.write = DeviceWrite,
};

static struct platform_info g_deviceMax6950 = {
	.tag = TAG_DEVICE_MAX6950,
	.fops = &g_fops,
	.states = 0,
};

static INT32 DeviceLedInit(void)
{
	if (DeviceInit() != SUCC) {
		return FAIL;
	}

	g_deviceMax6950.states |= DEVICE_INIT;
	RegisterDevice(&g_deviceMax6950);
	return SUCC;
}

module_init(DeviceLedInit);



