#include "worker.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>



int worker_init(worker* _Worker)
{
	memset(_Worker, 0, sizeof(worker));

	int i;
	for(i = 0; i < worker_max_tasks; i++)
	{
		_Worker->tasks[i].context = NULL;
		_Worker->tasks[i].callback = NULL;
	}

	return 0;
}

worker_task* worker_createTask(worker* _Worker, void* _Context, void (*_Callback)(void* _Context, uint64_t _MonTime))
{
	int i;
	for(i = 0; i < worker_max_tasks; i++)
	{
		if(_Worker->tasks[i].context == NULL && _Worker->tasks[i].callback == NULL)
		{
			_Worker->tasks[i].context = _Context;
			_Worker->tasks[i].callback = _Callback;
			return &_Worker->tasks[i];
		}
	}
	return NULL;
}
void worker_destroyTask(worker* _Worker, worker_task* _Task)
{
	if(_Task == NULL)
		return;

	int i;
	for(i = 0; i < worker_max_tasks; i++)
	{
		if(&_Worker->tasks[i] == _Task)
		{
			_Worker->tasks[i].context = NULL;
			_Worker->tasks[i].callback = NULL;
			break;
		}
	}
}

void worker_work(worker* _Worker, uint64_t _MonTime)
{
	int i;
	for(i = 0; i < worker_max_tasks; i++)
	{
		if(_Worker->tasks[i].callback != NULL)
			_Worker->tasks[i].callback(_Worker->tasks[i].context, _MonTime);

	}
}

int worker_getTaskCount(worker* _Worker)
{
	int counter = 0;
	int i;
	for(i = 0; i < worker_max_tasks; i++)
	{
		if(_Worker->tasks[i].callback != NULL)
			counter++;

	}

	return counter;
}

void worker_dispose(worker* _Worker)
{
	int i;
	for(i = 0; i < worker_max_tasks; i++)
	{
		_Worker->tasks[i].context = NULL;
		_Worker->tasks[i].callback = NULL;
	}
}


