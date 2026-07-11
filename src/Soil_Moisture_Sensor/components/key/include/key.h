#ifndef SOL_KEY_H
#define SOL_KEY_H

#include "task.h"

void key_init(void);
void key_proc(task_event_t ev);
void key_scan_proc(task_event_t ev);

#endif // SOL_KEY_H