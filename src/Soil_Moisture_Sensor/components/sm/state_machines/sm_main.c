#include "sm.h"
#include "sm_main.h"
#include "logger.h"

static const char * TAG = "SM_MAIN";

const char * sm_states_names_main[] = {
  "SM_MAIN_INIT",
};

void do_main_init(uint8_t from_func, uint8_t from_state, uint8_t to_func, uint8_t to_state, task_event_t ev)
{
  SOL_LOGD(TAG, "do_main_init");
}

static const sm_trans_t sm_trans_main_init[] = {
  {0, 0, 0, NULL}
};

const sm_trans_t * sm_trans_main[] = {
  sm_trans_main_init,
};
