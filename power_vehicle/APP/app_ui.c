#include "app_ui.h"
#include "app_computer.h"

#include "app_manager.h"
#include "app_task_define.h"
#include "init.h"
#include "stdio.h"
#include "queue.h"
#include "device_manager.h"	

static UINT16 g_dgusPage = 0xff;

static INT32 UiSendData(UINT8 desTag, void *data, UINT16 dataLen)
{
	return SUCC;
	return SendDataToQueue(TAG_APP_UI, desTag, data, dataLen);
}

static void UiStandaloneDataUpdate(UINT16 *uiData)
{
	struct dgus_addr_data dgus;
	struct platform_info *dev;
	UINT16 *displayData = uiData;

	if (GetDeviceInfo(TAG_DEVICE_UART_SCREEN, &dev) == FAIL) {
		return;
	}

	UINT16 *data = (UINT16 *)dgus.data;
	for (UINT16 i = DATA_RPM; i <= DATA_POWER; i++) {
		dgus.addr = i;
		*data = *(displayData++);
		dev->fops->ioctl(dev, DGUS_DATA_W_CMD, (void*)&dgus, sizeof(dgus.addr) + sizeof(UINT16));		
	}

}

static void UiHeartLungDataUpdate(UINT16 *uiData)
{
	struct dgus_addr_data dgus;
	struct platform_info *dev;
	UINT16 *displayData = uiData;

	if (GetDeviceInfo(TAG_DEVICE_UART_SCREEN, &dev) == FAIL) {
		return;
	}

	UINT16 *data = (UINT16 *)dgus.data;
	for (UINT16 i = DATA_RPM; i <= DATA_D5; i++) {
		dgus.addr = i;
		*data = *(displayData++);
		dev->fops->ioctl(dev, DGUS_DATA_W_CMD, (void*)&dgus, sizeof(dgus.addr) + sizeof(UINT16));		
	}
}

static void UiAppProcessUi(struct platform_info *app)
{
	UINT16 *uiData = (UINT16 *)app->private_data;

	switch(g_dgusPage) {
		case PAGE_STANDALONE:
			UiStandaloneDataUpdate(uiData);
			break;
		case PAGE_HEART_LUNG:
			UiHeartLungDataUpdate(uiData);
			break;
		default: break;
	}
}

static void UiAppProcess(struct platform_info *data)
{
	switch(data->tag) {
		case TAG_APP_COMPUTER:
			UiAppProcessUi(data);
			break;
		default:
		
			break;
	}
}

static bool DgusPackageAnalyze( UINT16 *pAddress, UINT16 *pKeyData, UINT8 *aucRxBuf )
{
	if((pAddress == NULL) || (pKeyData == NULL)) {
		return false;
	}

  UINT16 data=0;
	UINT16 address=0;

	if( DGUS_CMD_MEM_RD == aucRxBuf[DGUS_MEMORD_CMD] ) {
		address |= aucRxBuf[DGUS_MEMORD_ADDH];
		address <<= 8;
		address |= aucRxBuf[DGUS_MEMORD_ADDL];
		
		data |= aucRxBuf[DGUS_MEMORD_DATAH];
		data <<= 8;
		data |= aucRxBuf[DGUS_MEMORD_DATAL];
	}

	*pAddress = address;
	*pKeyData = data;

	return true;
}

static void StandaloneProcess(struct platform_info *dev)
{
	struct dgus_addr_data dgus;

	// clear data
	UINT16 *data = (UINT16 *)dgus.data;
	for (UINT16 i = DATA_RPM; i <= DATA_POWER; i++) {
		dgus.addr = i;
		*data = 0;
		dev->fops->ioctl(dev, DGUS_DATA_W_CMD, (void*)&dgus, sizeof(dgus.addr) + sizeof(UINT16));
	}

	// change to standalone page 
	g_dgusPage = PAGE_STANDALONE;
	dev->fops->ioctl(dev, DGUS_PAGE_W_CMD, (void*)&g_dgusPage, sizeof(g_dgusPage));
}

static void HeartLungProcess(struct platform_info *dev)
{
	struct dgus_addr_data dgus;

	// clear data
	UINT16 *data = (UINT16 *)dgus.data;
	for (UINT16 i = DATA_RPM; i <= DATA_D5; i++) {
		dgus.addr = i;
		*data = 0;
		dev->fops->ioctl(dev, DGUS_DATA_W_CMD, (void*)&dgus, sizeof(dgus.addr) + sizeof(UINT16));
	}

	// change to heart lung page 
	g_dgusPage = PAGE_HEART_LUNG;
	dev->fops->ioctl(dev, DGUS_PAGE_W_CMD, (void*)&g_dgusPage, sizeof(g_dgusPage));
}

static void VersionQueryProcess(struct platform_info *dev)
{
	struct dgus_addr_data dgus;

	// clear
	dgus.addr = TEXT_SOFTWARE_VER;
	memset(dgus.data, 0, sizeof(dgus.data));
	dev->fops->ioctl(dev, DGUS_TEXT_W_CMD, (void*)&dgus, sizeof(dgus));

	dgus.addr = TEXT_HARDWARE_VER;
	dev->fops->ioctl(dev, DGUS_TEXT_W_CMD, (void*)&dgus, sizeof(dgus));

	// write
	dgus.addr = TEXT_SOFTWARE_VER;
	char *data = (char *)dgus.data;
	memcpy(data, g_appSoftwareVersionBuf, strlen(g_appSoftwareVersionBuf));
	dev->fops->ioctl(dev, DGUS_TEXT_W_CMD, (void*)&dgus, sizeof(dgus.addr) + strlen(data));

	memset(dgus.data, 0, sizeof(dgus.data));
	dgus.addr = TEXT_HARDWARE_VER;
	memcpy(data, g_appHardwareVersionBuf, strlen(g_appHardwareVersionBuf));
	dev->fops->ioctl(dev, DGUS_TEXT_W_CMD, (void*)&dgus, sizeof(dgus.addr) + strlen(data));

	// change to version page 
	g_dgusPage = PAGE_VERSION;
	dev->fops->ioctl(dev, DGUS_PAGE_W_CMD, (void*)&g_dgusPage, sizeof(g_dgusPage));
}

static void HomeProcess(struct platform_info *dev)
{
	// change to home page 
	g_dgusPage = PAGE_HOME;
	dev->fops->ioctl(dev, DGUS_PAGE_W_CMD, (void*)&g_dgusPage, sizeof(g_dgusPage));
}

static void UiDeviceProcessUartScreen(struct platform_info *dev)
{
	UINT8* pdate = (UINT8 *)dev->private_data;
	UINT16 keyAddr = 0;
	UINT16 keyData = 0;
	if (!DgusPackageAnalyze(&keyAddr, &keyData, pdate)) {
		return;
	}

	switch( keyAddr ) {
		case KEY_RETURE_STANDALONE:
			StandaloneProcess(dev);
			break;
		case KEY_RETURE_HEART_LUNG:
			HeartLungProcess(dev);
			break;
		case KEY_RETURN_VER_QUERY:
			VersionQueryProcess(dev);
			break;
		case KEY_RETURN_HOME:
			HomeProcess(dev);
			break;
		default:
			break;
	}
	
	PrintfLogInfo(DEBUG_LEVEL, "[app_ui][UiDeviceProcessUartScreen]\n");
}

static void UiDeviceProcess(struct platform_info *data)
{
	struct platform_info *pData = data;
	switch(pData->tag) {
		case TAG_DEVICE_UART_SCREEN:
			UiDeviceProcessUartScreen(pData);
			break;
		default:
		
			break;
	}
}

void UiTask(void *pvParameters)
{
	struct platform_info queueData;
	QueueHandle_t xQueue = (QueueHandle_t)pvParameters;
	for(;;) {
		xQueueReceive(xQueue, &queueData, portMAX_DELAY);
		
		UINT8 cnt = uxQueueSpacesAvailable(xQueue);
		PrintfLogInfo(DEBUG_LEVEL, "[app_ui][UiTask] queue remain %d\n", cnt);

		if (queueData.tag < TAG_APP_END) {
			UiAppProcess(&queueData);
		} else if (queueData.tag >= TAG_DEVICE_START && queueData.tag < TAG_DEVICE_END) {
			UiDeviceProcess(&queueData);
		} else {
			// unknow tag error
		}
	}

}

static UINT8 g_uiData[COMPUTER_DATA_BUFF_SIZE] = { 0 };

static struct platform_info g_appUi = {
	.tag = TAG_APP_UI,
	.private_data = g_uiData,
};

static INT32 AppUiInit(void)
{
	static TaskHandle_t pUiTCB = NULL;
	TaskInit(g_appUi.tag, UiTask, &pUiTCB);
	RegisterApp(&g_appUi);
	return SUCC;
}

module_init(AppUiInit);



