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

static void FillSendBuff(char *pStr, char *sendBuff, UINT8 buffIndex, va_list pArgs)
{
  INT32 val, val1, powValue;
  UINT8 i, cnt;
  char *argStr;
  
	while(*pStr != '\0') {
			if (buffIndex >= PRINTF_BUFF_SIZE) {
				break;
			}
			
			if (*pStr == '%') {
				switch((char)*(++pStr)) {
					case 'c':
						sendBuff[buffIndex++] = va_arg(pArgs, int);
						break;
					case 'd':
						val = va_arg(pArgs, int);
						val1 = val;
					
						cnt = 0;
						while(val != 0) {
							cnt++;
							val /= 10;
						}
						for (i = 0; i < cnt; i++) {
							powValue = (INT32)pow(10, cnt-i-1);
							sendBuff[buffIndex++] = '0' + val1 / powValue;
							val1 %= powValue;
						}
						break;
					case 's':
						argStr = va_arg(pArgs, char*);
						for (i = 0; i < strlen(argStr); i++) {
							sendBuff[buffIndex++] = argStr[i];
						}
						break;
					default:
						sendBuff[buffIndex++] = '%';
						break;
				}
			} else {
				sendBuff[buffIndex++] = *pStr;
			}
			++pStr;
		}
}

void PrintfLogInfo(UINT8 level, char *str, ...)
{
	// return;
#if defined(PRINT_DEBUG_INFO) || defined(PRINT_WARNING_INFO) || defined(PRINT_ERROR_INFO)
	if (str == NULL) {
		return;
	}

	char *pStr = (char *)str;
  char *sendBuff = (char *)malloc(PRINTF_BUFF_SIZE);
  memset(sendBuff, 0, PRINTF_BUFF_SIZE);

	va_list pArgs;
	va_start(pArgs, str);

	
	switch(level) {
#if defined(PRINT_DEBUG_INFO)		
		case DEBUG_LEVEL:
			strcpy(sendBuff, DEBUG_INFO_STR);
			FillSendBuff(pStr, sendBuff, strlen(DEBUG_INFO_STR), pArgs);
			break;
#endif
#if defined(PRINT_WARNING_INFO)
		case WARNING_LEVEL:
			strcpy(sendBuff, WARNING_INFO_STR);
			FillSendBuff(pStr, sendBuff, strlen(WARNING_INFO_STR), pArgs);
			break;
#endif
#if defined(PRINT_ERROR_INFO)
		case ERROR_LEVEL:
			strcpy(sendBuff, ERROR_INFO_STR);
			FillSendBuff(pStr, sendBuff, strlen(ERROR_INFO_STR), pArgs);
			break;
#endif
		default:
			break;
	}

	taskENTER_CRITICAL();
  printf("%s\n", sendBuff);
	taskEXIT_CRITICAL();

	free(sendBuff);
  sendBuff = NULL;
	va_end(pArgs);
#else
	UNUSED(level);
	UNUSED(str);
#endif
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
