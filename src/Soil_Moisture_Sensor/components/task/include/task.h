#ifndef NEW_IV_CLOCK_TASK_H
#define NEW_IV_CLOCK_TASK_H

#include <stdbool.h>

// max 64
typedef enum _task_event_t
{           
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
  EV_LP,                 // 电池电量低
  EV_MP,                 // 电池充电到可以工作状态
  EV_CNT  
} task_event_t;

extern const char * task_names[];

typedef void (*TASK_PROC)(task_event_t);

void task_init (void);

void task_set(task_event_t ev);
bool task_test_clr(task_event_t ev);
void task_run(void);


#endif  // NEW_IV_CLOCK_TASK_H
