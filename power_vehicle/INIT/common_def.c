#include "string.h"
#include "stdlib.h"
#include "stdarg.h"
#include "math.h"

#include "common_def.h"
#include "app_manager.h"
#include "device_manager.h"
#include "freeRTOS.h"
#include "task.h"


struct platform_info_node *CommonInitList(void)
{
	struct platform_info_node *headNode = (struct platform_info_node *)malloc(sizeof(struct platform_info_node));
	if (headNode == NULL) {
		return NULL;
	}
	
	headNode->info = NULL;
	headNode->next = NULL;

	return headNode;
}

INT32 CommonInsertNodeToListTail(struct platform_info_node *headNode, struct platform_info *info)
{
	if (headNode == NULL) {
		return FAIL;
	}
	struct platform_info_node *deviceList = headNode;
	while(deviceList->next != NULL) {
		deviceList = deviceList->next;
	}
	struct platform_info_node *temp = (struct platform_info_node *)malloc(sizeof(struct platform_info_node));
	if (temp == NULL) {
		return FAIL;
	}

	temp->info = info;
	temp->next = NULL;
	
	deviceList->next = temp;
	return SUCC;
}

INT32 CommonDeleteNodeFromList(struct platform_info_node *headNode, struct platform_info *info)
{
	if (headNode == NULL) {
		return FAIL;
	}
	struct platform_info_node *frontNode = headNode;
	struct platform_info_node *rearNode = frontNode;

	while(rearNode->info != info && rearNode != NULL) {
		frontNode = rearNode;
		rearNode = rearNode->next;
	}

	if (rearNode == NULL) {
		return FAIL;
	}

	frontNode->next = rearNode->next;
	free(rearNode);

	return SUCC;
}

struct platform_info *GetPlatformInfo(UINT8 tag)
{
	struct platform_info *info = NULL;

	if (tag < TAG_APP_END) {
		GetAppInfo(tag, &info);
	} else if (tag >= TAG_DEVICE_START && tag < TAG_DEVICE_END) {
		GetDeviceInfo(tag, &info);
	} else {
		PrintfLogInfo(ERROR_LEVEL, "[common_def][GetPlatformInfo] tag=%d, error\n", tag);
	}
	
	return info;
}

void CommonTime(char *timeStr)
{
	UINT32 tick = xTaskGetTickCount();

	struct common_time *pTime = malloc(sizeof(struct common_time));
	memset(pTime, 0, sizeof(struct common_time));

	pTime->msec = tick % 1000;
	pTime->second = tick / 1000 % 60;
	pTime->minute = tick / 1000 / 60 % 60;
	pTime->hour = tick / 1000 / 60 / 60 % 24;
	sprintf(timeStr, "%02d-%02d-%02d:%03d ", pTime->hour, pTime->minute, pTime->second, pTime->msec);

	free(pTime);
	pTime = NULL;
}

char g_bspDebugBuffer[8192] = { 0 };
static char g_tickStr[COMMON_TICK_STR_SIZE] = { 0 };

void PrintfLogInfo(UINT8 level, char *str, ...)
{
#if defined(PRINT_DEBUG_INFO)

#elif defined(PRINT_WARNING_INFO)
	if (level < WARNING_LEVEL) {
		return;
	}
#elif defined(PRINT_ERROR_INFO)
	if (level < ERROR_LEVEL) {
		return;
	}
#endif

	if (str == NULL) {
		return;
	}

  char *sendBuff = (char *)malloc(PRINTF_BUFF_SIZE);
	if (sendBuff == NULL) {
		return;
	}
  memset(sendBuff, 0, PRINTF_BUFF_SIZE);

	va_list pArgs;
	va_start(pArgs, str);

	int infoSize = 0;
	
	switch(level) {
		case DEBUG_LEVEL:
			infoSize = sprintf(sendBuff, "%s", DEBUG_INFO_STR);
			break;
		case WARNING_LEVEL:
			infoSize = sprintf(sendBuff, "%s", WARNING_INFO_STR);
			break;
		case ERROR_LEVEL:
			infoSize = sprintf(sendBuff, "%s", ERROR_INFO_STR);
			break;
		default:
			free(sendBuff);
			sendBuff = NULL;
			va_end(pArgs);
			return;
			break;
	}

	vsprintf(sendBuff + infoSize, str, pArgs);

	BaseType_t schedulerState = xTaskGetSchedulerState();
	static int debugBuffOffset = 0;

	if (schedulerState == taskSCHEDULER_RUNNING) {
		SendDataToQueue(TAG_APP_DATA_STORAGE, TAG_APP_DATA_STORAGE, sendBuff, strlen(sendBuff));
		debugBuffOffset = 0;
	} else {
		if (debugBuffOffset < sizeof(g_bspDebugBuffer)) {
			CommonTime(g_tickStr);
			debugBuffOffset += sprintf(g_bspDebugBuffer + debugBuffOffset, "%s%s", g_tickStr, sendBuff);
		}
	}

	taskENTER_CRITICAL();
  printf("%s\n", sendBuff);
	taskEXIT_CRITICAL();

	free(sendBuff);
  sendBuff = NULL;
	va_end(pArgs);
}

#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

__attribute__((weak)) int _isatty(int fd)
{
    if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
        return 1;
 
    errno = EBADF;
    return 0;
}
 
__attribute__((weak)) int _close(int fd)
{
    if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
        return 0;
 
    errno = EBADF;
    return -1;
}
 
__attribute__((weak)) int _lseek(int fd, int ptr, int dir)
{
    (void)fd;
    (void)ptr;
    (void)dir;
 
    errno = EBADF;
    return -1;
}
 
__attribute__((weak)) int _fstat(int fd, struct stat *st)
{
    if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
    {
        st->st_mode = S_IFCHR;
        return 0;
    }
 
    errno = EBADF;
    return 0;
}

__attribute__((weak)) int _read(int file, char *ptr, int len)
{
    (void)file;
    int DataIdx;
    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
        // *ptr++ = __io_getchar();
		// *ptr++ = getchar();
    }
    return len;
}
 
__attribute__((weak)) int _write(int file, char *ptr, int len)
{
    (void)file;
    int DataIdx;

    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
        // __io_putchar(*ptr++);
		// fputc((int)*ptr++, (FILE *)file);
    }
    return len;
}
