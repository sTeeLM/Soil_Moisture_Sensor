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
bool power_recover_from_standby(void);
void power_enter_standby(uint32_t sec);
typedef enum _power_wakeup_reason_t
{
  POWER_WAKEUP_NONE = 0,
  POWER_WAKEUP_KEY,
  POWER_WAKEUP_ADP,
  POWER_WAKEUP_TIMEO
} power_wakeup_reason_t;

void power_check_wakeup_reason(void);
power_wakeup_reason_t power_get_wakeup_reason(void);

#endif // SOL_POWER_H