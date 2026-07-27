#include "key.h"
#include "logger.h"
#include "sm.h"
#include "task.h"
#include "clock.h"
#include "gpio_wrapper.h"
#include "delay.h"


#include "driver/gpio.h"
#include "driver/gpio_filter.h"

static const char * TAG = "KEY";

#define KEY_LPRESS_DELAY 2 // 长按时间
#define KEY_STATE_MASK 0x1
static bool key_down;
static bool key_lpress_send;
static uint32_t key_down_sec;

static void IRAM_ATTR key_isr_handler (void* param)
{
  delay_us(50);
  SOL_EARLY_LOGD(TAG, "key_isr_handler");
  task_set(EV_EX_INT);
}

void key_init(void)
{
  SOL_LOGI(TAG, "init");
  // 注册中断
  gpio_pin_glitch_filter_config_t glitch_cfg = {
    .gpio_num = GPIO_PIN_KEY, 
    .clk_src = GLITCH_FILTER_CLK_SRC_DEFAULT,
  };
  gpio_glitch_filter_handle_t glitch_handle;

  SOL_LOGI(TAG, "init");
  ESP_ERROR_CHECK(gpio_intr_enable(GPIO_PIN_KEY));
  ESP_ERROR_CHECK(gpio_set_intr_type(GPIO_PIN_KEY, GPIO_INTR_POSEDGE));
  ESP_ERROR_CHECK(gpio_isr_handler_add(GPIO_PIN_KEY, key_isr_handler, NULL));
  ESP_ERROR_CHECK(gpio_new_pin_glitch_filter(&glitch_cfg, &glitch_handle));
  ESP_ERROR_CHECK(gpio_glitch_filter_enable(glitch_handle));
}

void key_proc(task_event_t ev)
{
  switch (ev) {
    case EV_KEY_DOWN:
      SOL_LOGD(TAG, "key_proc EV_KEY_DOWN");  
      break;           
    case EV_KEY_PRESS:
      SOL_LOGD(TAG, "key_proc EV_KEY_PRESS");  
      break;
    case EV_KEY_LPRESS:
      SOL_LOGD(TAG, "key_proc EV_KEY_LPRESS");
      break; 
    case EV_KEY_UP:
      SOL_LOGD(TAG, "key_proc EV_KEY_UP");  
      break;  
    default:
      SOL_LOGW(TAG, "key_proc unknown event %d", ev);
      break;
  }
  sm_run(ev);
}

void key_scan_proc(task_event_t ev)
{
  if(gpio_wrapper_get_level(GPIO_PIN_KEY) == 1) {
    if(key_down == 0) {
      key_down = 1;
      task_set(EV_KEY_DOWN);
      key_down_sec = clock_get_now_sec();
    }
  } else {
    if(key_down == 1) {
      if(!key_lpress_send) {
        task_set(EV_KEY_PRESS);
      }
      key_down = 0;
      key_lpress_send = 0;
      task_set(EV_KEY_UP);
    }
  }

  if(key_down) {
    if(clock_diff_now_sec(key_down_sec) > KEY_LPRESS_DELAY) {
      task_set(EV_KEY_LPRESS);
      key_lpress_send = 1;
    }
  }

}