#include "smw.h"
#include <string.h>

smw g_smw;

int smw_init()
{
	memset(&g_smw, 0, sizeof(g_smw));

	int i;
	for(i = 0; i < smw_max_tasks; i++)
	{
		g_smw.tasks[i].context = NULL;
		g_smw.tasks[i].callback = NULL;
	}

	return 0;
}

smw_task* smw_createTask(void* _Context, void (*_Callback)(void* _Context, uint64_t _MonTime))
{
	int i;
	for(i = 0; i < smw_max_tasks; i++)
	{
		if(g_smw.tasks[i].context == NULL && g_smw.tasks[i].callback == NULL)
		{
			g_smw.tasks[i].context = _Context;
			g_smw.tasks[i].callback = _Callback;
			return &g_smw.tasks[i];
		}
	}

	return NULL;
}

void smw_destroyTask(smw_task* _Task)
{
	if(_Task == NULL)
		return;

	int i;
	for(i = 0; i < smw_max_tasks; i++)
	{
		if(&g_smw.tasks[i] == _Task)
		{
			g_smw.tasks[i].context = NULL;
			g_smw.tasks[i].callback = NULL;
			break;
		}
	}
}

void smw_work(uint64_t _MonTime)
{
	int i;
	for(i = 0; i < smw_max_tasks; i++)
	{
		if(g_smw.tasks[i].callback != NULL)
			g_smw.tasks[i].callback(g_smw.tasks[i].context, _MonTime);

	}
}

int smw_getTaskCount()
{
	int counter = 0;
	int i;
	for(i = 0; i < smw_max_tasks; i++)
	{
		if(g_smw.tasks[i].callback != NULL)
			counter++;

	}

	return counter;
}

void smw_dispose()
{
	int i;
	for(i = 0; i < smw_max_tasks; i++)
	{
		g_smw.tasks[i].context = NULL;
		g_smw.tasks[i].callback = NULL;
	}
}
