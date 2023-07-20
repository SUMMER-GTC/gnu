#include "app_data_storage.h"

#include "app_manager.h"
#include "app_task_define.h"
#include "init.h"
#include "stdio.h"
#include "queue.h"
#include "ff.h"

FATFS fs;													
FIL fnew;												
FRESULT res_sd;                
UINT fnum;            					  
BYTE ReadBuffer[1024] = {0};
BYTE WriteBuffer[] = "hello! this is test for fs! 你好！这是个文件系统测试！\r\n";

static void DataStorageProcess(struct platform_info *dev)
{
	UNUSED(dev);
	static UINT8 errState = 0;
	// mount fs
	res_sd = f_mount(&fs,"0:",1);

	if(res_sd == FR_NO_FILESYSTEM) {
		res_sd=f_mkfs("0:",0,0);		
		
		if(res_sd == FR_OK) {
			res_sd = f_mount(NULL,"0:",1);
			res_sd = f_mount(&fs,"0:",1);
		}
		else {
			errState = 1;
		}
	} else if(res_sd != FR_OK) {
		errState = 2;
  }
  else {
		errState = 3;
  }

	// write fs
	res_sd = f_open(&fnew, "0:FatFsTest.txt",FA_OPEN_ALWAYS | FA_WRITE );
	if ( res_sd == FR_OK ) {
		res_sd=f_write(&fnew,WriteBuffer,sizeof(WriteBuffer),&fnum);
    if(res_sd == FR_OK) {
			errState = 4;
    }
    else {
			errState = 5;
    }    

    f_close(&fnew);
	}
	else {	
		errState = 6;
	}

	// read
	res_sd = f_open(&fnew, "0:FatFsTest.txt", FA_OPEN_EXISTING | FA_READ); 	 
	if(res_sd == FR_OK) {
		res_sd = f_read(&fnew, ReadBuffer, sizeof(ReadBuffer), &fnum); 
    if(res_sd==FR_OK) {
			errState = 7;
    }
    else {
			errState = 8;
    }		
	}
	else {
		errState = 9;
	}
	f_close(&fnew);	
  
	f_mount(NULL,"0:",1);
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



