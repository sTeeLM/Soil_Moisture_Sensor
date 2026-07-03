#include <STC8G.H>

#include "system.h"
#include "com.h"
#include "delay.h"
#include "debug.h"
#include "clock.h"
#include "gpio.h"
#include "power.h"
#include "moisture.h"
#include "bt.h"
#include "adc.h"
#include "task.h"
#include "sm.h"

void main(void)
{
  EA = 1; // 全局开中断
  system_init();
  com_initialize();
  debug_initialize();
  clock_initialize();
  debug_onoff(1);
  
  gpio_initialize();
  adc_initialize(); // must before power
  
  power_initialize();
  
  if(!power_adapter_on()) {
    CDBG("power adapter NOT on, close serial");
    //gpio_set_mode(GPIO_SERIAL_RXD_PORT, GPIO_SERIAL_RXD_BIT, GPIO_MODE_IN);
    //gpio_set_mode(GPIO_SERIAL_TXD_PORT, GPIO_SERIAL_TXD_BIT, GPIO_MODE_IN);
    power_enable(true);
  } else {
    CDBG("power adapter on, close power_en");
    power_enable(false);
  }
  
  moisture_initialize();
  bt_initialize();
  debug_onoff(1);
  
  task_initialize();
  sm_initialize();
  
  while (1) {
    task_run();
  }
}