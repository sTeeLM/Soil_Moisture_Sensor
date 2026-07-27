#include "logger.h"
#include "mini_font.h"
#include "oled.h"
#include "delay.h"
#include "power.h"
#include "clock.h"
#include "key.h"
#include "hygrometer.h"
#include "gpio_wrapper.h"
#include "i2c_wrapper.h"
#include "adc_wrapper.h"

static const char *TAG = "MAIN";

void app_main(void)
{
  logger_init();
  delay_init();  

  // 获取唤醒状态
  if(power_recover_from_standby()) {
    power_check_wakeup_reason();
  }
  
  gpio_wrapper_init();
  i2c_wrapper_init();
  adc_wrapper_init();
  clock_init();
  power_init();
  key_init();
  hygrometer_init();
  oled_init();
  mini_font_init();

  power_enable(1);

  while(1) {
    task_run();
  }
}
