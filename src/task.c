#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "clock.h"
#include "task.h"
#include "debug.h"
#include "sm.h"

/* hardware*/
#include "button.h"


/*

  EV_INIT                = 0,
  EV_KEY_SCAN,           // 扫描按键
  EV_ADP_ON,             // adapter pluged on
  EV_ADP_OFF,            // adapter pluged off
  EV_250MS,              // 大约每250ms转一下
  EV_1S,                 // 大约每1s转一下 
  EV_KEY_DOWN,           // button按下 
  EV_KEY_PRESS,          // button键短按
  EV_KEY_LPRESS,         // button键长按
  EV_KEY_UP,             // mod抬起
  EV_TIMEO,              // 超时
  EV_LP,                 // bat low
  EV_FP,                 // bat full
  EV_SOL_ON,             // 太阳能板充电
  EV_SOL_OFF,            // 太阳能板没有充电
  EV_CNT  
  
*/

const char * code task_names[] =
{
  "EV_INIT",
  "EV_KEY_SCAN",  
  "EV_ADP_ON",
  "EV_ADP_OFF",
  "EV_250MS",
  "EV_1S",
  "EV_KEY_DOWN",  
  "EV_KEY_PRESS",
  "EV_KEY_LPRESS",
  "EV_KEY_UP",
  "EV_TIMEO",
  "EV_LP",
  "EV_FP",
  "EV_SOL_ON",
  "EV_SOL_OFF",
};

static void null_proc(enum task_events ev)
{
  sm_run(ev);
}


static const TASK_PROC task_procs[EV_CNT] = 
{
  null_proc,
  /* EV_SCAN_KEY */
  null_proc,
  /* EV_ADP_ON */
  null_proc,
  /* EV_ADP_OFF */
  null_proc,
  /* EV_250MS*/
  clock_time_proc,
  /* EV_1S*/
  clock_time_proc,
  /* EV_KEY_MOD_XX */
  button_proc,
  button_proc,
  button_proc,
  button_proc,
  /* EV_TIMEO */
  null_proc,
  /* EV_LP */
  null_proc,
  /* EV_FP */
  null_proc,
  /* EV_SOL_ON */
  null_proc,
  /* EV_SOL_OFF */
  null_proc,
};


uint16_t ev_bits0;
uint16_t ev_bits1;

void task_initialize (void)
{
  CDBG("task init");
  ev_bits0 = 0;
  ev_bits1 = 0;
}

void task_run(void)
{
  uint8_t c;
  for(c = 0; c < EV_CNT; c++) {
    if(task_test(c)) {
      task_clr(c);
      task_procs[c](c);
    }
  }
}

void task_dump(void)
{
  uint8_t i;
  for (i = 0 ; i < EV_CNT; i ++) {
    CDBG("[%02bd][%s] %c", i, task_names[i], task_test(i) ? '1' : '0');
  }
}
