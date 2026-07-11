#include <stdio.h>
#include "logger.h"
#include "mini_font.h"
#include "oled.h"
#include "delay.h"
#include "power.h"
#include "clock.h"
#include "key.h"
#include "gpio_wrapper.h"
#include "i2c_wrapper.h"

static const char *TAG = "MAIN";

void app_main(void)
{
  logger_init();
  delay_init();
  gpio_wrapper_init(0);
  i2c_wrapper_init();
  clock_init();
  power_init();
  key_init();
  oled_init();
  mini_font_init();
 
  while(1) {
    task_run();
  }
}
