#ifndef SOL_SM_MAIN_H
#define SOL_SM_MAIN_H

#include "task.h"
#include "sm.h"

extern const char * sm_states_names_main[];
extern const sm_trans_t * sm_trans_main[];

enum sm_states_main
{
  SM_MAIN_INIT,  
};

void do_main_init(uint8_t from_func, uint8_t from_state, uint8_t to_func, uint8_t to_state, task_event_t ev);

#endif  // SOL_SM_MAIN_H