#ifndef __SOL_SM_MAIN_H__
#define __SOL_SM_MAIN_H__

#include <stdint.h>

#include "task.h"

enum sm_states_main
{
  SM_MAIN_INIT, // 初始状态
  SM_MAIN_ADP_OFF_OLED_OFF, // 工作状态，USB未连接, OLED关闭
  SM_MAIN_ADP_OFF_OLED_ON, // 工作状态，USB未连接, OLED打开
  SM_MAIN_ADP_ON, // 工作状态，USB连接（OLED总是打开）
  SM_MAIN_LP_ADP_OFF, // 低电量状态，USB未连接（OLED总是打开）
  SM_MAIN_LP_ADP_ON, // 低电量状态，USB连接（OLED总是打开）
  SM_MAIN_POWER_OFF  // 关机
};

const struct sm_state_slot code sm_function_main[];

#endif
