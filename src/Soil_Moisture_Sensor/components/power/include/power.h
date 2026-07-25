#ifndef SOL_POWER_H
#define SOL_POWER_H
#include "hal/adc_ll.h"
#include <stdint.h>

#include "task.h"
void power_init(void);
void power_proc(task_event_t ev);
void power_enable(bool enable);
bool power_is_adapter_connected(void);
bool power_is_adapter_standby(void);
bool power_is_sol_on(void);
void power_dump(void);

#endif // SOL_POWER_H