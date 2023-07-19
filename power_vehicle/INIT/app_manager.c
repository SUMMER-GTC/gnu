#include "stdio.h"
#include "string.h"

#include "app_manager.h"
#include "app_task_define.h"
#include "app_ui.h"
#include "app_therapy.h"

static struct platform_info *g_appInfo[TAG_APP_END];

void RegisterApp(struct platform_info *app)
{
	if (app == NULL) {
		return;
	}
	g_appInfo[app->tag] = app;
}

INT32 GetAppInfo(UINT8 tag, struct platform_info **app)
{
	if (app == NULL) {
		return FAIL;
	}
	
	if (tag >= TAG_APP_END) {
		return FAIL;
	}

	*app = g_appInfo[tag];

	return SUCC;
}

INT32 SendDataToQueue(UINT8 srcTag, UINT8 desTag, void *data, UINT32 dataLen)
{
	if (data == NULL) {
		return FAIL;
	}
	
	if (srcTag >= TAG_APP_END) {
		return FAIL;
	}
	
	struct platform_info *info = NULL;
	GetAppInfo(srcTag, &info);
	if (info == NULL) {
		return FAIL;
	}

	switch (srcTag) {
		case TAG_APP_UI:
			if (dataLen > UI_DATA_BUFF_SIZE) {
				PrintfLogInfo(ERROR_LEVEL, "[app_manager][SendDataQueue] tag%d data buff overflow\n", TAG_APP_UI);
				return FAIL;
			}
			break;
		case TAG_APP_THERAPY:
			if (dataLen > THERAPY_DATA_BUFF_SIZE) {
				PrintfLogInfo(ERROR_LEVEL, "[app_manager][SendDataQueue] tag%d data buff overflow\n", TAG_APP_THERAPY);
				return FAIL;
			}
			break;
		default:
			break;
	}
	memcpy(info->private_data, data, dataLen);
	info->private_data_len = dataLen;
	
	SendDataToApp(SEND_FROM_NORMAL, desTag, info);
	
	return SUCC;
}


