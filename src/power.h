#ifndef __SOL_POWER_H__
#define __SOL_POWER_H__

#include <stdint.h>
#include <stdbool.h>

void power_initialize(void);
bool power_adapter_on(void);
uint16_t power_get_vol(void);
bool power_sol_on(void);
void power_enable(bool enable);
void power_dump(void);
#endif // __SOL_POWER_H__