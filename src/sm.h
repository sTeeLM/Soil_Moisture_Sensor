#ifndef __SOL_SM_H__
#define __SOL_SM_H__

#include <stdint.h>
#include <stddef.h>
#include "task.h"


enum sm_functions
{
  SM_MAIN  = 0,   // 主功能
};

typedef void (*SM_PROC)(uint8_t to_func, uint8_t to_state, enum task_events ev);


/* function: array of state */
/* state: array of trans */

struct sm_trans_slot
{
  enum task_events event;
  uint8_t to_function;
  uint8_t to_state;
  SM_PROC sm_proc;
};

struct sm_state_slot {
  const char * state_name;
  struct sm_trans_slot * state_trans_array;
};

struct sm_function_slot {
  const char * function_name;
  struct sm_state_slot * function_states_array;
};

extern uint8_t sm_cur_function;
extern uint8_t sm_cur_state;

void sm_initialize();

void sm_run(enum task_events ev);


#endif
