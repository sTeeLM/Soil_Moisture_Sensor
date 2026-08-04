#include "sm.h"
#include "sm_main.h"
#include "logger.h"
#include "task.h"

#include "oled.h"
#include "oled_ext.h"
#include "power.h"
#include "clock.h"
#include "ble_reporter.h"
#include "hygrometer.h"

static const char * TAG = "SM_MAIN";

const char * sm_states_names_main[] = {
  "SM_MAIN_ADP_OFF_OLED_OFF",
  "SM_MAIN_ADP_OFF_OLED_ON",
  "SM_MAIN_ADP_ON",
  "SM_MAIN_LP_ADP_OFF",
  "SM_MAIN_LP_ADP_ON",
  "SM_MAIN_PRE_POWER_OFF",
  "SM_MAIN_POWER_OFF"
};
static uint8_t sm_main_adp_on_off;
static void sm_main_draw_oled_on(bool adp_on)
{
  uint16_t mol = hygrometer_get_percent();
  uint16_t power = power_get_battery_voltage_percent();

  oled_clear();
  
  // 湿度
  if(mol == 1000) {
    oled_ext_draw_dig(0,0,1, false);
    oled_ext_draw_dig(20,0,0, false);
    oled_ext_draw_dig(40,0,0, false);
  } else {
    if(mol / 100)
      oled_ext_draw_dig(0,0,mol / 100, false);
    oled_ext_draw_dig(20,0,(mol % 100)/ 10, true);
    oled_ext_draw_dig(40,0,(mol % 10), false);
  }
  oled_ext_draw_percent(60, 0);

  // 电量，满电 or 没有插USB时，显示电量，否则闪烁显示电量最后一格，表示充电
  if((adp_on && power_is_adapter_standby()) || !adp_on) {
    oled_ext_draw_power(80, 16, 48, 16, power / 10, true);
  } else {
    oled_ext_draw_power(80, 16, 48, 16, power / 10, adp_on ? sm_main_adp_on_off : true);
    sm_main_adp_on_off = !sm_main_adp_on_off;
  }
  
  if(power_is_sol_on()) {
    oled_draw_bitmap(80, 0, 16, 16, OLE_EXT_ICON_SUN, OLED_DRAW_OVERWRITE);
  }
  
  if(power_is_adapter_connected()) {
    oled_draw_bitmap(96, 0, 16, 16, OLE_EXT_ICON_ADP, OLED_DRAW_OVERWRITE);
  }
  
  if(ble_reporter_is_enabled()) {
    oled_draw_bitmap(112, 0, 16, 16, OLE_EXT_ICON_BT, OLED_DRAW_OVERWRITE);
  }
  
  oled_redraw_buffer();

}

static void sm_main_draw_lp(bool adp_on)
{
  // 如果插入USB，显示充电状态，否则显示“低电量，关机”
  uint16_t power = power_get_battery_voltage_percent();
  oled_clear();
  if(adp_on) {
    oled_ext_draw_power(0, 0, 128, 32, power / 10, sm_main_adp_on_off);
    sm_main_adp_on_off = !sm_main_adp_on_off;
  } else {
    oled_ext_draw_wstring(0, 16, L"低电量,关机...", MINI_FONT_TYPE_ASCII_8X16, MINI_FONT_TYPE_CHINESE_16X16, OLED_DRAW_OVERWRITE);
  }
  oled_redraw_buffer();
}

static void sm_main_draw_poweroff(void)
{
  oled_clear();
  oled_ext_draw_wstring(0, 16, L"关机中...", MINI_FONT_TYPE_ASCII_8X16, MINI_FONT_TYPE_CHINESE_16X16, OLED_DRAW_OVERWRITE);
  oled_redraw_buffer();
}

// 总周期 = SM_MAIN_STANDBY_SEC + SM_MAIN_REPORT_SEC
#define  SM_MAIN_STANDBY_SEC  50  // 睡眠时间
#define  SM_MAIN_REPORT_SEC   10  // 维持蓝牙上报事件
#define  SM_MAIN_OLED_ON_SEC  10  // OLED点亮时间
#define  SM_MAIN_POWER_OFF_WAIT_SEC 2  // 低电量 or 按下按键后，等待关机的时间

static bool sm_main_reporting; 

// 既然插上了USB，电力是无限的，我们可以肆无忌惮地一直打开OLED和一直上报数据
// EV_ADP_ON: 插入USB唤醒
// EV_MP:     从低电量充电到可以工作的电量
// EV_INIT:   插入USB，并且初始启动
// EV_1S:     刷新
// EV_TIMEO:  定时信号
static void do_main_adp_on(uint8_t from_func, uint8_t from_state, uint8_t to_func, uint8_t to_state, task_event_t ev)
{
  if(ev == EV_ADP_ON || ev == EV_INIT || ev == EV_MP) {
    oled_enable(true);
    sm_main_reporting = false;
    clock_set_timer(SM_MAIN_STANDBY_SEC);
    power_probe();
    sm_main_draw_oled_on(true);
  } else if(ev == EV_1S) {
    power_probe();
    sm_main_draw_oled_on(true);
  } else if(ev == EV_TIMEO) {
    power_probe();
    if(sm_main_reporting) {
      // 停止蓝牙
      ble_reporter_enable(false);
      sm_main_reporting = false;
      clock_set_timer(SM_MAIN_STANDBY_SEC);
    } else {
      // 启动蓝牙，并采集、上报数据
      ble_reporter_set_data(hygrometer_get_percent(), power_get_battery_voltage_percent());
      ble_reporter_enable(true);
      sm_main_reporting = true;
      clock_set_timer(SM_MAIN_REPORT_SEC);
    }
  }
}

// 仅仅在开机之后，或者是按下按键唤醒之后，或者是拔下USB之后进入这个短暂的状态，不需要上报数据，仅仅点亮OLED，刷新读数
// EV_KEY_PRESS: 按下按键唤醒，或者是在点亮状态又按了一下
// EV_INIT: 按下按键启动
// EV_1S: 定时刷新
// EV_ADP_OFF: 拔下USB
static void do_main_adp_off_oled_on(uint8_t from_func, uint8_t from_state, uint8_t to_func, uint8_t to_state, task_event_t ev)
{
  if(ev == EV_KEY_PRESS || ev == EV_INIT || ev == EV_ADP_OFF) {
    oled_enable(true);
    ble_reporter_enable(false);
    if(ev == EV_INIT) {
      sm_main_reporting = false;
    }
    clock_set_timer(SM_MAIN_OLED_ON_SEC);
    power_probe();
    sm_main_draw_oled_on(false);
  } else if(ev == EV_1S) {
    power_probe();
    sm_main_draw_oled_on(false);
  }
}

// 从SM_MAIN_ADP_OFF_OLED_ON超时进入这个状态
// 注意：
//   1. 正常工作时，长时间停留在这个状态
//   2. EV_TIMEO有可能是clock触发超时，或者是从deep sleep 状态唤醒进入
// EV_TIMEO: 超时进入这个状态
static void do_main_adp_off_oled_off(uint8_t from_func, uint8_t from_state, uint8_t to_func, uint8_t to_state, task_event_t ev)
{
  SOL_LOGD(TAG, "do_main_adp_off_oled_off: from_state %s, ev %s", sm_states_names_main[from_state], task_names[ev]);
  if(ev == EV_TIMEO) {
    if(from_state == SM_MAIN_ADP_OFF_OLED_ON) {
      oled_enable(false);
      sm_main_reporting = false;
      ble_reporter_enable(false);
      power_enter_standby(SM_MAIN_STANDBY_SEC);
      // 不会运行到这里
    } else {
      if(sm_main_reporting) {
        // 停止蓝牙
        ble_reporter_enable(false);
        sm_main_reporting = false;
        power_enter_standby(SM_MAIN_STANDBY_SEC);
        // 不会运行到这里
      } else {
        power_probe();
        // 启动蓝牙，并采集、上报数据
        ble_reporter_set_data(hygrometer_get_percent(), power_get_battery_voltage_percent());
        ble_reporter_enable(true);
        sm_main_reporting = true;
        clock_set_timer(SM_MAIN_REPORT_SEC);
      }
    }
  }
}

// 由于低电量，同时没有插USB，准备关机
// EV_LP
static void do_main_lp_adp_off(uint8_t from_func, uint8_t from_state, uint8_t to_func, uint8_t to_state, task_event_t ev)
{
  oled_enable(true);
  ble_reporter_enable(false);
  sm_main_draw_lp(false);
  clock_set_timer(SM_MAIN_POWER_OFF_WAIT_SEC); // 在这个时间之后，就关机了
}

// 电量低，且插上了USB，显示充电画面
static void do_main_lp_adp_on(uint8_t from_func, uint8_t from_state, uint8_t to_func, uint8_t to_state, task_event_t ev)
{
  if(ev == EV_ADP_ON) {
    oled_enable(true);
    ble_reporter_enable(false);
    sm_main_draw_lp(true);
  } else if(ev == EV_1S) {
    power_probe();
    sm_main_draw_lp(true);
  }
}

static void do_main_pre_power_off(uint8_t from_func, uint8_t from_state, uint8_t to_func, uint8_t to_state, task_event_t ev)
{
  oled_enable(true);
  ble_reporter_enable(false);
  sm_main_draw_poweroff();
  clock_set_timer(SM_MAIN_POWER_OFF_WAIT_SEC); // 在这个时间之后，就关机了
}

static void do_main_power_off(uint8_t from_func, uint8_t from_state, uint8_t to_func, uint8_t to_state, task_event_t ev)
{
  power_enable(false);
  // 不会运行到这里
}

static const sm_trans_t sm_trans_main_adp_off_oled_off[] = {
  {EV_ADP_ON, SM_MAIN, SM_MAIN_ADP_ON, do_main_adp_on},
  {EV_KEY_PRESS, SM_MAIN, SM_MAIN_ADP_OFF_OLED_ON, do_main_adp_off_oled_on},
  {EV_TIMEO, SM_MAIN, SM_MAIN_ADP_OFF_OLED_OFF, do_main_adp_off_oled_off},
  {EV_LP, SM_MAIN, SM_MAIN_LP_ADP_OFF, do_main_lp_adp_off},
  {0, 0, 0, NULL}
};

static const sm_trans_t sm_trans_main_adp_off_oled_on[] = {
  {EV_INIT, SM_MAIN, SM_MAIN_ADP_OFF_OLED_ON, do_main_adp_off_oled_on},
  {EV_KEY_LPRESS, SM_MAIN, SM_MAIN_PRE_POWER_OFF, do_main_pre_power_off},
  {EV_1S, SM_MAIN, SM_MAIN_ADP_OFF_OLED_ON, do_main_adp_off_oled_on},
  {EV_ADP_ON, SM_MAIN, SM_MAIN_ADP_ON, do_main_adp_on},
  {EV_KEY_PRESS, SM_MAIN, SM_MAIN_ADP_OFF_OLED_ON, do_main_adp_off_oled_on},
  {EV_TIMEO, SM_MAIN, SM_MAIN_ADP_OFF_OLED_OFF, do_main_adp_off_oled_off},
  {EV_LP, SM_MAIN, SM_MAIN_LP_ADP_OFF, do_main_lp_adp_off},
  {0, 0, 0, NULL}
};

static const sm_trans_t sm_trans_main_adp_on[] = {
  {EV_INIT, SM_MAIN, SM_MAIN_ADP_ON, do_main_adp_on},
  {EV_1S, SM_MAIN, SM_MAIN_ADP_ON, do_main_adp_on},
  {EV_TIMEO, SM_MAIN, SM_MAIN_ADP_ON, do_main_adp_on},
  {EV_ADP_OFF, SM_MAIN, SM_MAIN_ADP_OFF_OLED_ON, do_main_adp_off_oled_on},
  {0, 0, 0, NULL}
};

static const sm_trans_t sm_trans_main_lp_adp_off[] = {
  {EV_TIMEO, SM_MAIN, SM_MAIN_POWER_OFF, do_main_power_off},
  {EV_ADP_ON, SM_MAIN, SM_MAIN_LP_ADP_ON, do_main_lp_adp_on},
  {0, 0, 0, NULL}
};

static const sm_trans_t sm_trans_main_lp_adp_on[] = {
  {EV_1S, SM_MAIN, SM_MAIN_LP_ADP_ON, do_main_lp_adp_on},
  {EV_MP, SM_MAIN, SM_MAIN_ADP_ON, do_main_adp_on},
  {0, 0, 0, NULL}
};

static const sm_trans_t sm_trans_main_pre_power_off[] = {
  {EV_TIMEO, SM_MAIN, SM_MAIN_POWER_OFF, do_main_power_off},
  {0, 0, 0, NULL}
};

static const sm_trans_t sm_trans_main_power_off[] = {
  {0, 0, 0, NULL}
};

const sm_trans_t * sm_trans_main[] = {
  sm_trans_main_adp_off_oled_off,
  sm_trans_main_adp_off_oled_on,
  sm_trans_main_adp_on,
  sm_trans_main_lp_adp_off,
  sm_trans_main_lp_adp_on,
  sm_trans_main_pre_power_off,
  sm_trans_main_power_off
};
