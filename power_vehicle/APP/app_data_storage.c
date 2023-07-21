#include "app_data_storage.h"

#include "app_manager.h"
#include "app_task_define.h"
#include "init.h"
#include "stdio.h"
#include "queue.h"
#include "ff.h"

#define LOG_PATH "0:/log/debug_log.txt"
#define LOGICAL_DRIVE_NUMBER "0:"

#define DEBUG_FILE_SIZE (1024 * 1024 * 8)  // 8M bytes
#define DATA_STORAGE_WRITE_BUFFER_SIZE (50)

__packed struct data_storage_file {
	UINT32 writePointer;
	UINT32 fileTotalSize;
	UINT8 writeBuffer[DATA_STORAGE_WRITE_BUFFER_SIZE];
	UINT16 writeBufferLen;
};

static struct data_storage_file g_dataStorageFile = {
	.writePointer = 0,
	.fileTotalSize = DEBUG_FILE_SIZE,
	.writeBuffer = { 0 },
	.writeBufferLen = 0,
};

static FATFS g_fs;								
static FIL g_file;

static BYTE ReadBuffer[1024] = {0};

static INT32 MoutFs(void)
{
	FRESULT res;

	res = f_mount(&g_fs, LOGICAL_DRIVE_NUMBER, 1);
	if(res == FR_NO_FILESYSTEM) {
		res = f_mkfs(LOGICAL_DRIVE_NUMBER, 0, 0);
		
		if(res != FR_OK) {
			return FAIL;
		}
		res = f_mount(NULL, LOGICAL_DRIVE_NUMBER, 1);
		if (res != FR_OK) {
				return FAIL;
		}
		res = f_mount(&g_fs, LOGICAL_DRIVE_NUMBER, 1);
		if (res != FR_OK) {
				return FAIL;
		}
	} else if (res == FR_NOT_READY) {
		return FAIL;
	} else if (res != FR_OK) {
		return FAIL;
	}

	return SUCC;
}

static void FsWriteData(UINT8 *data, UINT16 dataLen)
{
	FRESULT res;
	FILINFO fileInfo;

	if (g_dataStorageFile.writeBufferLen + dataLen < DATA_STORAGE_WRITE_BUFFER_SIZE) {
		memcpy(g_dataStorageFile.writeBuffer + g_dataStorageFile.writeBufferLen, data, dataLen);
		g_dataStorageFile.writeBufferLen += dataLen;
		return;
	}

	if (MoutFs() != SUCC) {
		return;
	}

	res = f_open(&g_file, LOG_PATH, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
	if (res != FR_OK) {
		return;
	}

	f_lseek(&g_file, g_dataStorageFile.writePointer);

	UINT fnum;
	res = f_write(&g_file, g_dataStorageFile.writeBuffer, g_dataStorageFile.writeBufferLen, &fnum);
	if (res != FR_OK) {
		return;
	}
	g_dataStorageFile.writeBufferLen = 0;
	g_dataStorageFile.writePointer += fnum;

	f_lseek(&g_file, f_tell(&g_file));
	// write over 4kB data
	res = f_write(&g_file, data, dataLen, &fnum);
	if (res != FR_OK) {
		return;
	}
	g_dataStorageFile.writePointer += fnum;

	if (g_dataStorageFile.writePointer > g_dataStorageFile.fileTotalSize) {
		g_dataStorageFile.writePointer = 0;
	}

	res = f_close(&g_file);
	if (res != FR_OK) {
		return;
	}
}

static void DataStorageProcess(struct platform_info *dev)
{
	UNUSED(dev);
	UINT8 data[] = "0123456789";	

	for (UINT32 i = 0; i < 10; i++) {
		FsWriteData(data, sizeof(data));
	}
}

void DataStorageTask(void *pvParameters)
{
	struct platform_info queueData;
	QueueHandle_t xQueue = (QueueHandle_t)pvParameters;
	for(;;) {
		xQueueReceive(xQueue, &queueData, portMAX_DELAY);
		
		DataStorageProcess(&queueData);
	}

}

static struct platform_info g_appDataStorage = {
	.tag = TAG_APP_DATA_STORAGE,
};

static INT32 AppDataStorageInit(void)
{
	static TaskHandle_t pdataStorageTCB = NULL;
	TaskInit(g_appDataStorage.tag, DataStorageTask, &pdataStorageTCB);
	RegisterApp(&g_appDataStorage);
	return SUCC;
}

module_init(AppDataStorageInit);



