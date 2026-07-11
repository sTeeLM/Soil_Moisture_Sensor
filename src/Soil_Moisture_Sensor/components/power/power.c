#include "power.h"
#include "logger.h"
#include "sm.h"

static const char * TAG = "POWER";

void power_init(void)
{
  SOL_LOGI(TAG, "init");
}

void power_proc(task_event_t ev)
{
  sm_run(ev);
}