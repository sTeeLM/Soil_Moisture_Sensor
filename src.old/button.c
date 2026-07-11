#include <STC8G.H>

#include "button.h"
#include "cext.h"
#include "task.h"
#include "debug.h"
#include "sm.h"
#include "delay.h"
#include "gpio.h"
#include "clock.h"
#include "power.h"
#include "gpio.h"

#define KEY_LPRESS_DELAY 3 // 长按时间
#define KEY_STATE_MASK 0x1
static bool key_down;
static bool key_lpress_send;
static uint32_t key_down_sec;


void button_initialize(void)
{
  CDBG("button init");
  key_down = 0;
  key_lpress_send = 0;
  IT0 = 0; //使能INT0 上升沿和下降沿中断
  EX0 = 1; //使能INT0 中断  
}

static void button_isr() interrupt 0 using 2
{
  task_set(EV_EX_INT);
}

void button_scan_proc(enum task_events ev)
{
  if(GPIO_KEY_INT) {
    if(key_down == 0) {
      key_down = 1;
      task_set(EV_KEY_DOWN);
      key_down_sec = clock_get_now_sec();
    }
  } else {
    if(key_down == 1) {
      if(!key_lpress_send) {
        task_set(EV_KEY_PRESS);
      }
      key_down = 0;
      key_lpress_send = 0;
      task_set(EV_KEY_UP);
    }
  }
  
 
  if(key_down) {
    if(clock_diff_now_sec(key_down_sec) > KEY_LPRESS_DELAY) {
      task_set(EV_KEY_LPRESS);
      key_lpress_send = 1;
    }
  }
}


void button_proc(enum task_events ev)
{
  switch (ev) {
    case EV_KEY_DOWN:
      CDBG("button_scan_proc EV_KEY_DOWN");  
      break;           
    case EV_KEY_PRESS:
      CDBG("button_scan_proc EV_KEY_PRESS");  
      break;
    case EV_KEY_LPRESS:
      CDBG("button_scan_proc EV_KEY_LPRESS");
      break; 
    case EV_KEY_UP:
      CDBG("button_scan_proc EV_KEY_UP");  
      break;  
    default:
      ;
  }
  
  sm_run(ev);
}
