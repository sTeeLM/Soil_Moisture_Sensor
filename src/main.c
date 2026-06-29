#include <STC8G.H>

#include "system.h"
#include "com.h"
#include "delay.h"
#include "debug.h"
#include "clock.h"
#include "gpio.h"
#include "power.h"
#include "moisture.h"
#include "task.h"
#include "sm.h"

void main(void)
{
  EA = 1; // 全局开中断
  system_init();
  com_initialize();
  debug_initialize();
  
  gpio_initialize();
  power_initialize();
  moisture_initialize();
  
  debug_onoff(1);
  
  clock_initialize();
  task_initialize();
  sm_initialize(0);
  
  while (1) {
    CDBG("this is a test");
    delay_ms(500);
  }
}