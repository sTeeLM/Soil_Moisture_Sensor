#ifndef __SOL_POWER_H__
#define __SOL_POWER_H__

#include <stdint.h>
#include <stdbool.h>

#include "task.h"

void power_initialize(void);
bool power_adapter_on(void);
bool power_chrg_on(void);
bool power_stdby_on(void);
bool power_sol_on(void);

uint16_t power_get_vol(void);
uint16_t power_get_vol_percent(void);

void power_enable(bool enable);
void power_dump(void);
void power_probe(void);
void power_proc(enum task_events ev);

void power_enable_lvo(bool enable);
bool power_sleep(void);

#endif // __SOL_POWER_H__