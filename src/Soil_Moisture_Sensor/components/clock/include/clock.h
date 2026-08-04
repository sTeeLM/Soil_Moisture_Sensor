#ifndef SOL_CLOCK_H
#define SOL_CLOCK_H

#include "task.h"

#include <stdint.h>

void clock_init(void);
void clock_time_proc(task_event_t ev);

uint32_t clock_get_now_sec(void);
uint32_t clock_diff_now_sec(uint32_t start_sec);
void clock_set_timer(uint32_t sec);
#endif // SOL_CLOCK_H