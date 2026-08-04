#ifndef SOL_SM_MAIN_H
#define SOL_SM_MAIN_H

#include "task.h"
#include "sm.h"

extern const char * sm_states_names_main[];
extern const sm_trans_t * sm_trans_main[];

enum sm_states_main
{
  SM_MAIN_ADP_OFF_OLED_OFF, // 工作状态，USB未连接, OLED关闭
  SM_MAIN_ADP_OFF_OLED_ON,  // 工作状态，USB未连接, OLED打开
  SM_MAIN_ADP_ON,           // 工作状态，USB连接（OLED总是打开）
  SM_MAIN_LP_ADP_OFF,       // 低电量状态，USB未连接（OLED总是打开）
  SM_MAIN_LP_ADP_ON,        // 低电量状态，USB连接（OLED总是打开）
  SM_MAIN_PRE_POWER_OFF,    // 通过按键准备关机
  SM_MAIN_POWER_OFF         // 关机
};

#endif  // SOL_SM_MAIN_H