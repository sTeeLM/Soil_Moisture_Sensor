#ifndef SOL_CLOCK_H
#define SOL_CLOCK_H

#include "task.h"

void clock_init(void);
void clock_time_proc(task_event_t ev);

#endif // SOL_CLOCK_H