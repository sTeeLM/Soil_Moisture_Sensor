#ifndef SOL_CLOCK_SM_H
#define SOL_CLOCK_SM_H

#include <stdint.h>
#include <stdbool.h>

#include "task.h"

typedef enum _sm_functions_t 
{
  SM_MAIN,                 // 主功能：时间日期展示，传感器数据展示  
  SM_FUNCTION_CNT
} sm_functions_t;


typedef void (*SM_PROC)(uint8_t from_func, uint8_t from_state, uint8_t to_func, uint8_t to_state, task_event_t ev);

typedef struct _sm_trans_t
{
  uint8_t event;
  uint8_t to_function;
  uint8_t to_state;
  SM_PROC sm_proc;
} sm_trans_t;

void sm_init(void);

void sm_run(task_event_t ev);

#endif  // SOL_CLOCK_SM_H
