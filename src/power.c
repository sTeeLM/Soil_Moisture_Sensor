#include <STC8G.H>

#include "power.h"
#include "debug.h"
#include "gpio.h"
#include "adc.h"
#include "delay.h"
#include "task.h"
#include "sm.h"
#include "cext.h"

static bit power_saved_chrg;
static bit power_saved_stdby;
static uint16_t power_vol;
static uint16_t power_saved_vol;

static bit power_int_flag;

#define POWER_LP_THRESHOLD 3000 // 3.0V
#define POWER_MP_THRESHOLD 3300 // 3.3V
#define POWER_FP_THRESHOLD 4200 // 4.2V

static void power_isr_stdby (void) interrupt 10 using 2
{
  task_set(EV_EX_INT);
}

static void power_isr_chrg (void) interrupt 11 using 2
{
  task_set(EV_EX_INT);
}

static uint16_t power_get_vol_internal(void);
void power_initialize(void)
{
  CDBG("power init");
  power_saved_chrg = 0;
  power_saved_stdby = 0;
  power_vol = power_get_vol_internal();
  power_saved_vol = power_vol;
  
  // enable int2 int3
  INTCLKO |= 0x30;
  ELVD = 0;
  power_int_flag = 0;  
}

void power_enable_lvo(bool enable)
{
  PCON &= ~0x20; // clear LVDF
  ELVD = enable;
}

// 如果由于按键、插电唤醒，返回true
// 否则返回false
bool power_sleep(void)
{
  task_clr(EV_EX_INT);
  power_enable_lvo(0);
  CDBG("enter power_sleep\n");
  power_int_flag = 1;
  PCON |= 0x2; // into PD
  CDBG("leave power_sleep\n");
  if(task_test(EV_EX_INT)) {
    CDBG("clear power_int_flag\n");
    power_int_flag = 0; // 按键和插电都会清除这个flag
  }  
  power_enable_lvo(1);
  return !power_int_flag;   
}

bool power_adapter_on(void)
{
  if(GPIO_POWER_CHRG == 0 && GPIO_POWER_STDBY == 0)
    return false;
  
  return true;
}

static bool power_adapter_prev_on(void)
{
  if(power_saved_chrg == 0 && power_saved_stdby == 0)
    return false;
  
  return true;
}

bool power_chrg_on(void)
{
  return GPIO_POWER_CHRG == 0;
}

bool power_stdby_on(void)
{
  return GPIO_POWER_STDBY == 0;
}

bool power_sol_on(void)
{
  return GPIO_POWER_SOL != 0;
}

void power_enable(bool enable)
{
  GPIO_POWER_EN = enable ? 1 : 0;
}

static uint16_t power_get_vol_internal(void)
{
  int32_t ref_val;
  int32_t power_val;
  int16_t vol;
  
  GPIO_POWER_VOL_EN = 1;
  
  delay_10us(100); // 等待buffer电容充电
   
  ref_val = (int32_t) adc_get_val(15);
  power_val = (int32_t) adc_get_val(7);
  
  GPIO_POWER_VOL_EN = 0;
 
  // power_val-> vol
  // ref_val  -> 1185 mV
  // 求电源电压 vol
  // 电源电压 vol = 1185 * power_val / ref_val;
  vol = (uint16_t)(((int32_t)1185 * power_val) / ref_val);
  return vol;
}


uint16_t power_get_vol_percent(void)
{
  int16_t ret;
  if(power_adapter_on() && power_stdby_on()) {
    ret = 1000;
  } else if(power_vol <= POWER_LP_THRESHOLD) {
    ret = 0;
  } else {
    ret = cext_linear_interpolate_short(POWER_LP_THRESHOLD, 0, POWER_FP_THRESHOLD, 1000, power_vol);
  }
  if(ret > 1000)
    ret = 1000;
  if(ret < 0)
    ret = 0;
  return (uint8_t)ret;
}

void power_proc(enum task_events ev)
{
  switch(ev) {
    case EV_ADP_ON:  CDBG("EV_ADP_ON"); break;
    case EV_ADP_OFF: CDBG("EV_ADP_OFF"); break;
    case EV_LP:      CDBG("EV_LP"); break;
    case EV_MP:      CDBG("EV_MP"); break;
  }
  sm_run(ev);
}

void power_probe(void)
{
  uint16_t power_vol_temp;
  power_dump();
  
  if(!power_adapter_prev_on() && power_adapter_on()) {
    task_set(EV_ADP_ON);
  } else if(power_adapter_prev_on() && !power_adapter_on()) {
    task_set(EV_ADP_OFF);
  }
  
  power_saved_chrg  = GPIO_POWER_CHRG;
  power_saved_stdby = GPIO_POWER_STDBY;
  
  power_vol_temp = power_get_vol_internal();
  if(power_vol_temp > POWER_FP_THRESHOLD)
    power_vol_temp = POWER_FP_THRESHOLD;
  power_vol = cext_iir_uint16(power_vol, power_vol_temp, 32);
  if(power_vol <= POWER_LP_THRESHOLD && power_saved_vol > POWER_LP_THRESHOLD) {
    task_set(EV_LP);
  } else if(power_vol >= POWER_MP_THRESHOLD && power_saved_vol < POWER_MP_THRESHOLD) {
    task_set(EV_MP);
  }
  
  power_saved_vol = power_vol;
}

uint16_t power_get_vol(void)
{
  return power_vol;
}

void power_dump(void)
{
  CDBG("CHRG(%bu):%bu, STDBY(%bu):%bu, SOL:%bu, VOL:%u mV(%bd%%)", 
    power_saved_chrg ? 1 : 0,
    GPIO_POWER_CHRG ? 1 : 0, 
    power_saved_stdby ? 1 : 0,
    GPIO_POWER_STDBY ? 1 : 0, 
    GPIO_POWER_SOL ? 1 : 0,
    power_get_vol(), power_get_vol_percent());
}