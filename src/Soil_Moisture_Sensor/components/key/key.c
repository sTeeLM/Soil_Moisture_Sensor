#include "key.h"
#include "logger.h"
#include "sm.h"

static const char * TAG = "KEY";

void key_init(void)
{
  SOL_LOGI(TAG, "init");
}

void key_proc(task_event_t ev)
{
  sm_run(ev);
}

void key_scan_proc(task_event_t ev)
{
  sm_run(ev);
}