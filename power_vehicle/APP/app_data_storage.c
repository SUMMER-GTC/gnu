#include "app_data_storage.h"

#include "app_manager.h"
#include "app_task_define.h"
#include "init.h"
#include "stdio.h"
#include "queue.h"
#include "ff.h"

#define LOGICAL_DRIVE_NUMBER "0:"

#define LOG_PATH "0:/log/debug_log.txt"
#define LOG_DIR "0:/log"

#define POWER_SPEED_PATH "0:/data/power_speed.txt"
#define POWER_SPEED_DIR "0:/data"

#define DEBUG_LOG_FILE_SIZE (1024 * 1024 * 8)  // 8M bytes
#define POWER_SPEED_FILE_SIZE (1024 * 1024 * 8)  // 8M bytes
#define WRITE_BUFFER_SIZE (1024)
#define WRITE_POINTER_STR_SIZE 16

struct data_storage_file {
	UINT32 writePointer;
	char writePointerStr[WRITE_POINTER_STR_SIZE];
	UINT32 fileTotalSize;
	UINT8 writeBuffer[WRITE_BUFFER_SIZE];
	UINT16 writeBufferLen;
} __packed;

static struct data_storage_file g_debugLogFile = {
	.writePointer = WRITE_POINTER_STR_SIZE,
	.writePointerStr = { 0 },
	.fileTotalSize = DEBUG_LOG_FILE_SIZE,
	.writeBuffer = { 0 },
	.writeBufferLen = 0,
};

static struct data_storage_file g_powerSpeedFile = {
	.writePointer = WRITE_POINTER_STR_SIZE,
	.writePointerStr = { 0 },
	.fileTotalSize = POWER_SPEED_FILE_SIZE,
	.writeBuffer = { 0 },
	.writeBufferLen = 0,
};

static FATFS g_fs;

static FIL g_logFile;
static FIL g_powerAndSpeedFile;

static UINT8 g_receiveData[128] = { 0 };

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

static void FsWriteLogData(char *data, UINT16 dataLen, struct data_storage_file *dataStorage, 
														FIL *file, const char *path, const char *dir)
{
	FRESULT res;

	if (dataStorage->writeBufferLen + dataLen <= WRITE_BUFFER_SIZE) {
		memcpy(dataStorage->writeBuffer + dataStorage->writeBufferLen, data, dataLen);
		dataStorage->writeBufferLen += dataLen;
		return;
	}

	if (MoutFs() != SUCC) {
		return;
	}

	res = f_open(file, path, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
	if (res == FR_NO_PATH) {
		res = f_mkdir(dir);
		if (res != FR_OK) {
			return;
		}
		f_open(file, path, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
	} else if (res != FR_OK) {
		return;
	}

	UINT br;
	if (dataStorage->writePointer == WRITE_POINTER_STR_SIZE) {
		res = f_read(file, dataStorage->writePointerStr, WRITE_POINTER_STR_SIZE, &br);
		if (res != FR_OK) {
			return;
		}
		int writePointer = atoi(dataStorage->writePointerStr);
		if (writePointer > WRITE_POINTER_STR_SIZE) {
			dataStorage->writePointer = (UINT32)writePointer;
		}
	}

	f_lseek(file, dataStorage->writePointer);

	UINT fnum;
	res = f_write(file, dataStorage->writeBuffer, dataStorage->writeBufferLen, &fnum);
	if (res != FR_OK) {
		return;
	}
	dataStorage->writeBufferLen = 0;
	dataStorage->writePointer += fnum;

	f_lseek(file, f_tell(file));
	// write over 4kB data
	res = f_write(file, data, dataLen, &fnum);
	if (res != FR_OK) {
		return;
	}
	dataStorage->writePointer += fnum;

	if (dataStorage->writePointer > dataStorage->fileTotalSize) {
		dataStorage->writePointer = WRITE_POINTER_STR_SIZE;
	}

	sprintf(dataStorage->writePointerStr, "%015ld", dataStorage->writePointer);
	dataStorage->writePointerStr[WRITE_POINTER_STR_SIZE - 1] = '\n';

	f_lseek(file, 0);
	res = f_write(file, dataStorage->writePointerStr, WRITE_POINTER_STR_SIZE, &fnum);
	if (res != FR_OK) {
		return;
	}

	res = f_close(file);
	if (res != FR_OK) {
		return;
	}
}

extern char g_bspDebugBuffer[8192];

static void FsLogSave(UINT8 *data, UINT16 dataLen)
{
	char *buff = malloc(COMMON_TICK_STR_SIZE + dataLen);
	memset(buff, 0, COMMON_TICK_STR_SIZE + dataLen);

	CommonTime(buff);
	UINT8 offset = strlen(buff);
	memcpy(buff + offset, data, dataLen);

	if (strlen(g_bspDebugBuffer) != 0) {
		FsWriteLogData(g_bspDebugBuffer, strlen(g_bspDebugBuffer), &g_debugLogFile, &g_logFile, LOG_PATH, LOG_DIR);
		memset(g_bspDebugBuffer, 0, sizeof(g_bspDebugBuffer));
	}

	FsWriteLogData(buff, strlen(buff), &g_debugLogFile, &g_logFile, LOG_PATH, LOG_DIR);

	free(buff);
	buff = NULL;
}

static void FsPowerSpeedSave(UINT8 *data, UINT16 dataLen)
{
	char *buff = malloc(COMMON_TICK_STR_SIZE + dataLen);
	memset(buff, 0, COMMON_TICK_STR_SIZE + dataLen);

	CommonTime(buff);
	UINT8 offset = strlen(buff);
	memcpy(buff + offset, data, dataLen);

	FsWriteLogData(buff, strlen(buff), &g_powerSpeedFile, &g_powerAndSpeedFile, POWER_SPEED_PATH, POWER_SPEED_DIR);

	free(buff);
	buff = NULL;
}

static void DataStorageProcess(struct platform_info *app)
{
	UNUSED(app);
	static UINT32 power = 0;
	static UINT32 speed = 0;

	if (app->tag == TAG_APP_DATA_STORAGE) {
		FsLogSave(app->private_data, app->private_data_len);
	}

	++power;
	++speed;

	UINT8 powerSpeedData[16];
	sprintf((char *)powerSpeedData, "%6ld, %6ld\n", power, speed);
	FsPowerSpeedSave(powerSpeedData, strlen((char *)powerSpeedData));
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
	.private_data = g_receiveData,
	.private_data_len = 0,
};

static INT32 AppDataStorageInit(void)
{
	static TaskHandle_t pdataStorageTCB = NULL;
	TaskInit(g_appDataStorage.tag, DataStorageTask, &pdataStorageTCB);
	RegisterApp(&g_appDataStorage);
	return SUCC;
}

module_init(AppDataStorageInit);



