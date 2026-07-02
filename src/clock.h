#ifndef __SOL_CLOCK_H__
#define __SOL_CLOCK_H__


#include <stdint.h>
#include "task.h"

typedef struct _clock_struct_t
{
  uint32_t hour;   // 0 - 23
  uint8_t min;    // 0 - 59
  uint8_t sec;    // 0 - 59
  uint8_t ms625;   // 0 - 511
} clock_struct_t;

uint32_t clock_get_hour(void);
uint8_t clock_get_min(void);
uint8_t clock_get_sec(void);
uint8_t clock_get_ms625(void);

void clock_initialize(void);
void clock_time_proc(enum task_events ev);
uint32_t clock_get_now_sec(void);
uint32_t clock_diff_now_sec(uint32_t sec);
void clock_set_timer(uint8_t timer_sec);
uint32_t clock_get_now_jiff(void);
uint32_t clock_diff_now_jiff(uint32_t saved_jiff);
#endif