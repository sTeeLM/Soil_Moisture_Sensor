#include "task.h"
#include "logger.h"
#include "sm.h"
#include "clock.h"
#include "key.h"
#include "power.h"


#include <string.h>
#include <stdatomic.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

static const char * TAG = "TASK";
/*
  EV_INIT                = 0,
  EV_EX_INT,             // 外部中断
  EV_KEY_SCAN,           // 扫描按键
  EV_ADP_ON,             // adapter pluged on
  EV_ADP_OFF,            // adapter pluged off
  EV_1S,                 // 大约每1s转一下
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
const char * task_names[EV_CNT] =
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

static void null_proc(task_event_t ev)
{
  sm_run(ev);
}

static const TASK_PROC task_procs[EV_CNT] = 
{
  null_proc,
  /* EV_EX_INT */
  null_proc,
  /* EV_SCAN_KEY */
  key_scan_proc,
  /* EV_ADP_ON */
  power_proc,
  /* EV_ADP_OFF */
  power_proc,
  /* EV_250MS*/
  clock_time_proc,
  /* EV_1S*/
  clock_time_proc,
  /* EV_KEY_MOD_XX */
  key_proc,
  key_proc,
  key_proc,
  key_proc,
  /* EV_TIMEO */
  null_proc,
  /* EV_LP */
  power_proc,
  /* EV_MP */
  power_proc

};

#define  TASK_MAX_EVENT_CNT 64

static atomic_uint_least64_t ev_bits;

// 原子 Test and Clear：将指定位置 0，并返回其原本的状态（1->true, 0->false）
static bool task_bitmask_test_and_clear(atomic_uint_least64_t *bitmap, int bit_index) 
{
    uint64_t mask = ~(1ULL << bit_index);
    // atomic_fetch_and 将目标位置 0，并返回修改前的旧值
    uint64_t old_val = atomic_fetch_and(bitmap, mask);
    // 检查旧值中该位是否为 1
    return (old_val & (1ULL << bit_index)) != 0;
}

// 原子 Set：将指定位置 1（无条件覆盖）
static void task_bitmask_set(atomic_uint_least64_t *bitmap, int bit_index) 
{
    uint64_t mask = (1ULL << bit_index);
    // 使用按位或操作置 1
    atomic_fetch_or(bitmap, mask);
}

void task_init (void)
{
  SOL_LOGI(TAG, "init");
  atomic_store(&ev_bits, 0);
}

void task_run(void)
{
  uint8_t c;
  for(c = 0; c < EV_CNT; c++) {
    if(task_test_clr(c)) {
      task_procs[c](c);
    }
  }
}

void task_set(task_event_t ev)
{
  task_bitmask_set(&ev_bits, ev);
}


bool task_test_clr(task_event_t ev)
{
  return task_bitmask_test_and_clear(&ev_bits, ev);
}
