#include "sm.h"
#include "logger.h"
#include "task.h"

#include "sm_main.h"

static const char * TAG = "SM";

// 状态机结构：
// 按照大的功能分块为function，每一个function又细分为多个state
// 可以认为，一个16位整数唯一决定了一个状态机状态，高8位是function，低8位是state
// 这种设计，比一个打平的state状态机，在不使用hash下效率更高，因为大部分情况状态流转都在一个function之内
// 同时也更容易模块化

// 不同function之间，尽量通过do_xxx_init这个单一的入口跳转，让耦合降到最低

static const char * sm_functions_names[] = {
  "SM_MAIN",            // 主功能  
};

static const char ** sm_states_names[] = {
  sm_states_names_main
};

static const sm_trans_t ** sm_trans_table[] = {
  sm_trans_main
};

static uint8_t sm_cur_fuction;
static uint8_t sm_cur_state;

void sm_init(void)
{
  SOL_LOGI(TAG, "init");
  /*
  sm_cur_fuction[SM_AUX_CORE_ID] = SM_SENSOR;
  sm_cur_state[SM_AUX_CORE_ID]   = SM_SENSOR_INIT;

  sm_cur_fuction[SM_APP_CORE_ID] = SM_CLOCK;
  sm_cur_state[SM_APP_CORE_ID]   = SM_CLOCK_INIT;

  task_set_cpu(SM_APP_CORE_ID, EV_EC11_UP);
  task_set_cpu(SM_AUX_CORE_ID, EV_EC11_UP);
  */
}

void sm_run(task_event_t ev)
{
  const sm_trans_t *p = NULL;
  
  p = sm_trans_table[sm_cur_fuction][sm_cur_state];
  while(p != NULL && p->sm_proc) {
    if(p->event == ev) {
      SOL_LOGD(TAG, "[%s][%s][%s] -> [%s][%s]",
        task_names[ev],
        sm_functions_names[sm_cur_fuction],
        sm_states_names[sm_cur_fuction][sm_cur_state],
        sm_functions_names[p->to_function],
        sm_states_names[p->to_function][p->to_state]     
      );
      p->sm_proc(sm_cur_fuction, sm_cur_state, p->to_function, p->to_state, ev);
      sm_cur_fuction = p->to_function;
      sm_cur_state   = p->to_state;
      break;
    }
    p ++;
  }
}

