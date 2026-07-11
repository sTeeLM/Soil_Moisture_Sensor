#ifndef SOL_POWER_H
#define SOL_POWER_H
#include "hal/adc_ll.h"
#include <stdint.h>

#include "task.h"
void power_init(void);
void power_proc(task_event_t ev);
#endif // SOL_POWER_H