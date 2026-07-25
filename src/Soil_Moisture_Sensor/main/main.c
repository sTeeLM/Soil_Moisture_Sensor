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
  gpio_wrapper_init();
  i2c_wrapper_init();
  clock_init();
  power_init();
  key_init();
  oled_init();
  mini_font_init();

  if(power_is_adapter_connected()) {
    SOL_LOGI(TAG, "adapter connected");

  } else {
    SOL_LOGI(TAG, "adapter not connected");
  }

  oled_enable(true);
  oled_fill_rect(0, 0, 20, 20, true);
  oled_redraw_buffer();
  while(1) {
    //task_run();
    delay_ms(1000);
    power_dump();
  }
}
