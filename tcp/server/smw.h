#ifndef __smw_h_
#define __smw_h_

#include <stdint.h>

#ifndef smw_max_tasks
	#define smw_max_tasks 16
#endif

typedef struct
{
	void* context;
	void (*callback)(void* context, uint64_t monTime);

} smw_task;


typedef struct
{
	smw_task tasks[smw_max_tasks];

} smw;

extern smw g_smw;

int smw_init();

smw_task* smw_createTask(void* _Context, void (*_Callback)(void* _Context, uint64_t _MonTime));
void smw_destroyTask(smw_task* _Task);

void smw_work(uint64_t _MonTime);

int smw_getTaskCount();

void smw_dispose();

#endif /*__smw_h*/
