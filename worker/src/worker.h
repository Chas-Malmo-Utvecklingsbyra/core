#ifndef __WORKER_H__
#define __WORKER_H__

/* COORDINATING WORKER */

#include <stdint.h>

#ifndef worker_max_tasks
	#define worker_max_tasks 16
#endif

typedef struct
{
	void* context;
	void (*callback)(void* context, uint64_t monTime);

} worker_task;

typedef struct
{
	worker_task tasks[worker_max_tasks];

} worker;


int worker_init(worker* _Worker);

worker_task* worker_createTask(worker* _Worker, void* _Context, void (*_Callback)(void* _Context, uint64_t _MonTime));
void worker_destroyTask(worker* _Worker, worker_task* _Task);

void worker_work(worker* _Worker, uint64_t _MonTime);

int worker_getTaskCount(worker* _Worker);

void worker_dispose(worker* _Worker);


#endif