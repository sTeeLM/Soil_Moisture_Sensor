#ifndef __SOL_BUTTON_H__
#define __SOL_BUTTON_H__

#include "cext.h"
#include "task.h"

#include <stdint.h>
#include <stdbool.h>

void button_initialize(void);
void button_scan_proc(enum task_events ev);
void button_proc(enum task_events ev);
#endif
