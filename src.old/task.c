#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "clock.h"
#include "task.h"
#include "debug.h"
#include "sm.h"

/* hardware*/
#include "button.h"
#include "power.h"

/*
  EV_INIT                = 0,
  EV_EX_INT,             // 外部中断
  EV_KEY_SCAN,           // 扫描按键
  EV_ADP_ON,             // adapter pluged on
  EV_ADP_OFF,            // adapter pluged off
  EV_1S,                 // 大约每1S转一下
  EV_10S,                // 大约每10s转一下 
  EV_KEY_DOWN,           // button按下 
  EV_KEY_PRESS,          // button键短按
  EV_KEY_LPRESS,         // button键长按
  EV_KEY_UP,             // mod抬起
  EV_TIMEO,              // 超时
  EV_LP,                 // bat low
  EV_MP,                 // bat can work
  EV_CNT  
*/

const char * code task_names[] =
{
  "EV_INIT",
  "EV_EX_INT",
  "EV_KEY_SCAN",  
  "EV_ADP_ON",
  "EV_ADP_OFF",
  "EV_1S",
  "EV_10S",
  "EV_KEY_DOWN",  
  "EV_KEY_PRESS",
  "EV_KEY_LPRESS",
  "EV_KEY_UP",
  "EV_TIMEO",
  "EV_LP",
  "EV_MP"
};

static void null_proc(enum task_events ev)
{
  sm_run(ev);
}


static const TASK_PROC task_procs[EV_CNT] = 
{
  null_proc,
  /* EV_EX_INT */
  null_proc,
  /* EV_SCAN_KEY */
  button_scan_proc,
  /* EV_ADP_ON */
  power_proc,
  /* EV_ADP_OFF */
  power_proc,
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
  power_proc,
  /* EV_LP */
  power_proc,
  /* EV_MP */
  power_proc
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
