#include "sm_main.h"
#include "task.h"
#include "sm.h"
#include "cext.h"
#include "debug.h"
#include "power.h"
#include "gpio.h"
#include "delay.h"
#include "oled.h"
#include "tdc.h"
#include "power.h"
#include "oled.h"
#include "oled_font.h"
#include "oled_icon.h"
#include "oled_ext.h"
#include "moisture.h"
#include "bt.h"

#define SM_MAIN_REPORT_TEST_CNT 4  // 每隔多少个15S上报数据一次？
static uint8_t report_test_cnt;     

#define SM_MAIN_LP_DRAW_CNT 4 // 关机之前显示低电量/BYE几秒？
static uint8_t lp_draw_cnt;

// 显示湿度、电量、是否插电、是否太阳能充电
static bool on_off;
static void oled_draw(bool charging)
{
  uint16_t mol = moisture_get_data();
  uint16_t power = power_get_vol_percent() / 10;
  oled_clear();
  
  // 湿度
  if(mol == 1000) {
    oled_draw_dig(0,0,1, false);
    oled_draw_dig(20,0,0, false);
    oled_draw_dig(40,0,0, false);
  } else {
    if(mol / 100)
      oled_draw_dig(0,0,mol / 100, false);
    oled_draw_dig(20,0,(mol % 100)/ 10, true);
    oled_draw_dig(40,0,(mol % 10), false);
  }
  oled_draw_bitmap(60, 0, 20, 32, MINI_FONT_ASCII_PERCENT, OLED_DRAW_OVERWRITE);
  
  // 电量
  if(power == 100) {
    oled_draw_power(80, 16, 48, 16, 100, true);
  } else {
    oled_draw_power(80, 16, 48, 16, power, charging ? on_off : true);
    on_off = !on_off;
  }
  
  if(power_sol_on()) {
    oled_draw_bitmap(80, 0, 16, 16, MINI_FONT_ICON_SUN, OLED_DRAW_OVERWRITE);
  }
  
  if(power_adapter_on())
    oled_draw_bitmap(96, 0, 16, 16, MINI_FONT_ICON_ADP, OLED_DRAW_OVERWRITE);
  
  if(bt_enabled()) {
    oled_draw_bitmap(112, 0, 16, 16, MINI_FONT_ICON_BT, OLED_DRAW_OVERWRITE);
  }
  
  oled_redraw_buffer();
}

// 显示“低电量关机”
static void oled_draw_lp(void)
{
  oled_clear();
  
  oled_redraw_buffer();
}

// 显示 ByeBye（正常关机）
static void oled_draw_bye(void)
{
  oled_clear();

  oled_redraw_buffer();
}

// 显示大大的充电标记（充电中，电量过低，无法正常工作）
static void oled_draw_charging(void)
{
  uint16_t power = power_get_vol_percent() / 10;
  oled_clear();
  if(power == 100) {
    oled_draw_power(0, 0, 128, 32, 100, true);
  } else {
    oled_draw_power(0, 0, 128, 32, power, on_off);
    on_off = !on_off;
  }
  oled_redraw_buffer();
}

// 通过蓝牙上报数据
static void report_data(void)
{
  uint16_t power = power_get_vol_percent();
  uint16_t mol = moisture_get_data();
  
  bt_report_data(power, mol);
}

static void do_main_init(uint8_t to_func, uint8_t to_state, enum task_events ev)
{
  report_test_cnt   = 0;
  lp_draw_cnt = 0;
  power_probe();
  moisture_probe();
}

static void do_main_adp_off_oled_off(uint8_t to_func, uint8_t to_state, enum task_events ev)
{
  if(ev == EV_TIMEO) {
    if(sm_cur_state == SM_MAIN_ADP_OFF_OLED_OFF) { // 上报数据，睡眠
      if((report_test_cnt % SM_MAIN_REPORT_TEST_CNT) == 0) {
        power_probe();
        moisture_probe();
        report_data();  // 开启蓝牙广播
      } else if((report_test_cnt % SM_MAIN_REPORT_TEST_CNT) == 1) {
        bt_enable(false); // 第一次醒来，关闭广播
      }
      tdc_trigger_sleep(TDC_TYPE_15S);
      report_test_cnt ++;
    } else if (sm_cur_state == SM_MAIN_ADP_OFF_OLED_ON){ // 关闭屏幕，睡眠
      report_test_cnt = 0;
      oled_enable(false);
      bt_enable(false);
      tdc_trigger_sleep(TDC_TYPE_15S);
    }
  }
}

static void do_main_adp_off_oled_on(uint8_t to_func, uint8_t to_state, enum task_events ev)
{
  if(ev == EV_1S) {
    power_probe();
    moisture_probe();
    oled_draw(false);
  } else if(ev == EV_KEY_PRESS) {
    oled_enable(true);
    power_probe();
    moisture_probe();
    oled_draw(false);
    tdc_trigger(TDC_TYPE_15S); //按下按键之后，设置定时器，超时就会关屏幕
  } else if(ev == EV_ADP_OFF) {
    oled_enable(true);
    power_probe();
    moisture_probe();
    oled_draw(false);
    tdc_trigger(TDC_TYPE_15S); //拔出USB，设置定时器，就会关屏幕
  }
}

static void do_main_adp_on(uint8_t to_func, uint8_t to_state, enum task_events ev)
{
  if(ev == EV_ADP_ON || ev == EV_1S) {
    oled_enable(true);
    power_probe();
    moisture_probe();
    oled_draw(true);
    if(ev == EV_1S) {
      if((report_test_cnt % (SM_MAIN_REPORT_TEST_CNT * 15)) == 0) {
        report_data();  // 开启蓝牙广播
      } else if((report_test_cnt % (SM_MAIN_REPORT_TEST_CNT * 15)) == 15) {
        //bt_enable(false); // 15s后关闭广播
      }
      report_test_cnt ++;
    }
  } else if(ev == EV_MP || ev == EV_INIT) {
    power_probe();
    moisture_probe();
    oled_draw(true);
  }
}

static void do_main_lp_adp_off(uint8_t to_func, uint8_t to_state, enum task_events ev)
{
  if(ev == EV_LP) {
    oled_enable(true);
    oled_draw_lp();
    lp_draw_cnt = 0;
  } else if(ev == EV_1S) {
    oled_draw_lp();
    lp_draw_cnt ++;
    if(lp_draw_cnt >= SM_MAIN_LP_DRAW_CNT) {
      power_enable(false);
    }
  }
}

static void do_main_lp_adp_on(uint8_t to_func, uint8_t to_state, enum task_events ev)
{
  if(ev == EV_1S) {
    power_probe();
    oled_draw_charging();
  }
}

static void do_main_power_off(uint8_t to_func, uint8_t to_state, enum task_events ev)
{
  if(ev == EV_KEY_PRESS) {
    oled_enable(true);
    oled_draw_bye();
    lp_draw_cnt = 0;
  } else if(ev == EV_1S) {
    oled_draw_bye();
    lp_draw_cnt ++;
    if(lp_draw_cnt >= SM_MAIN_LP_DRAW_CNT) {
      power_enable(false);
    }
  }
}

static const struct sm_trans_slot code  sm_trans_main_init[] = {
  {EV_INIT, SM_MAIN, SM_MAIN_ADP_OFF_OLED_ON, do_main_init},
  {NULL, NULL, NULL, NULL}
};

static const struct sm_trans_slot code  sm_trans_main_adp_off_oled_off[] = {
  // 从深睡中醒过来，读取传感器，并上报数据
  {EV_TIMEO, SM_MAIN, SM_MAIN_ADP_OFF_OLED_OFF, do_main_adp_off_oled_off}, 
  // 低电量了, 该关机了
  {EV_LP, SM_MAIN, SM_MAIN_LP_ADP_OFF, do_main_lp_adp_off},
  // 插上USB了, 点亮屏幕
  {EV_ADP_ON, SM_MAIN, SM_MAIN_ADP_ON, do_main_adp_on},
  // 按键被按下, 点亮屏幕
  {EV_KEY_PRESS, SM_MAIN, SM_MAIN_ADP_OFF_OLED_ON, do_main_adp_off_oled_on},
  // 长按关机
  {EV_KEY_LPRESS, SM_MAIN, SM_MAIN_POWER_OFF, do_main_power_off},
  {NULL, NULL, NULL, NULL}
};

static const struct sm_trans_slot code  sm_trans_main_adp_off_oled_on[] = {
  // 超时了，该关屏幕了
  {EV_TIMEO, SM_MAIN, SM_MAIN_ADP_OFF_OLED_OFF, do_main_adp_off_oled_off}, 
  // 刷新屏幕，并上报数据
  {EV_1S, SM_MAIN, SM_MAIN_ADP_OFF_OLED_ON, do_main_adp_off_oled_on}, 
  // 低电量了, 该关机了
  {EV_LP, SM_MAIN, SM_MAIN_LP_ADP_OFF, do_main_lp_adp_off},
  // 插上USB了
  {EV_ADP_ON, SM_MAIN, SM_MAIN_ADP_ON, do_main_adp_on},
  // 长按关机
  {EV_KEY_LPRESS, SM_MAIN, SM_MAIN_POWER_OFF, do_main_power_off},
  {NULL, NULL, NULL, NULL}
};

static const struct sm_trans_slot code  sm_trans_main_adp_on[] = {
  // 刷新屏幕， 并上报数据
  {EV_1S, SM_MAIN, SM_MAIN_ADP_ON, do_main_adp_on},
  // 拔下USB，保持屏幕亮一会
  {EV_ADP_OFF, SM_MAIN, SM_MAIN_ADP_OFF_OLED_ON, do_main_adp_off_oled_on},
  {NULL, NULL, NULL, NULL}
};

static const struct sm_trans_slot code  sm_trans_main_lp_adp_off[] = {
  // 刷新屏幕，并关闭电源
  {EV_1S, SM_MAIN, SM_MAIN_LP_ADP_OFF, do_main_lp_adp_off},
  {NULL, NULL, NULL, NULL}
};

static const struct sm_trans_slot code  sm_trans_main_lp_adp_on[] = {
  // 刷新屏幕， 暂停上报数据
  {EV_1S, SM_MAIN, SM_MAIN_LP_ADP_ON, do_main_lp_adp_on},
  // 电池充到可以工作的状态了
  {EV_MP, SM_MAIN, SM_MAIN_ADP_ON, do_main_adp_on},
  // 拔下USB，就关闭电源
  {EV_ADP_OFF, SM_MAIN, SM_MAIN_LP_ADP_OFF, do_main_lp_adp_off},
  {NULL, NULL, NULL, NULL}
};

static const struct sm_trans_slot code  sm_trans_main_power_off[] = {
  {EV_1S, SM_MAIN, SM_MAIN_POWER_OFF, do_main_power_off},
  {NULL, NULL, NULL, NULL}
};

const struct sm_state_slot code sm_function_main[] = {
  {"SM_INIT", sm_trans_main_init},
  {"SM_ADP_OFF_OLED_OFF", sm_trans_main_adp_off_oled_off},
  {"SM_ADP_OFF_OLED_ON", sm_trans_main_adp_off_oled_on}, 
  {"SM_ADP_ON", sm_trans_main_adp_on}, 
  {"SM_LP_ADP_OFF", sm_trans_main_lp_adp_off},
  {"SM_LP_ADP_ON", sm_trans_main_lp_adp_on},  
  {"SM_POWER_OFF", sm_trans_main_power_off}
};