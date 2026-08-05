#include "logger.h"
#include "clock.h"
#include "driver/gptimer.h"

#include "power.h"
#include "sm.h"
#include "task.h"

static const char * TAG = "CLOCK";

static uint64_t clock_ticks;
static uint32_t clock_timer_saved_sec;
static uint32_t clock_timer_diff_sec;

static bool IRAM_ATTR clock_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    // 处理事件回调的一般流程：
    // 1. 从 user_ctx 中拿到用户上下文数据（需事先从 gptimer_register_event_callbacks 中传入）
    // 2. 从 edata 中获取警报事件数据，比如 edata->count_value
    // 3. 执行用户自定义操作
    // 4. 返回上述操作期间是否有高优先级的任务被唤醒了，以便通知调度器做切换任务

    clock_ticks ++;

    if(clock_ticks % 25 == 0) {
      if(clock_ticks % 100 == 0) {
        task_set(EV_1S);
        if((clock_timer_diff_sec != 0 && clock_diff_now_sec(clock_timer_saved_sec) > clock_timer_diff_sec)) {
          clock_timer_diff_sec = 0;
          task_set(EV_TIMEO);
        }
      }
      if(clock_ticks % 1000 == 0) {
        task_set(EV_10S);
      }
    }

    // 每秒扫描20次按键
    if(clock_ticks % 5 == 0) {
      task_set(EV_KEY_SCAN);
    }

    return false;
}

void clock_init(void)
{
  // 设置时钟中断
  gptimer_handle_t gptimer = NULL;
  gptimer_config_t timer_config = {
      .clk_src = GPTIMER_CLK_SRC_DEFAULT, // 选择默认的时钟源
      .direction = GPTIMER_COUNT_UP,      // 计数方向为向上计数
      .resolution_hz = 1 * 1000 * 1000,   // 分辨率为 1 MHz，即 1 次滴答为 1 微秒
  };
  gptimer_alarm_config_t alarm_config = {
    .reload_count = 0,      // 当警报事件发生时，定时器会自动重载到 0
    .alarm_count = 10000,     // 设置实际的警报周期，因为分辨率是 10ms
    .flags.auto_reload_on_alarm = true, // 使能自动重载功能
  };

  // 创建定时器实例
  gptimer_event_callbacks_t cbs = {
    .on_alarm = clock_cb, // 当警报事件发生时，调用用户回调函数
  };
  SOL_LOGI(TAG, "init");
  
  ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));
  // 设置定时器的警报动作
  ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));
  // 注册定时器事件回调函数，允许携带用户上下文
  ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));
  // 使能定时器
  ESP_ERROR_CHECK(gptimer_enable(gptimer));
  // 启动定时器
  ESP_ERROR_CHECK(gptimer_start(gptimer));

  clock_timer_saved_sec = clock_timer_diff_sec = 0;

}

void clock_time_proc(task_event_t ev)
{
  sm_run(ev);
}

uint32_t clock_get_now_sec(void)
{
  return clock_ticks / 100;
}
uint32_t clock_diff_now_sec(uint32_t start_sec)
{
  uint32_t now_sec = clock_get_now_sec();
  return (uint32_t)(now_sec - start_sec);
}

void clock_set_timer(uint32_t sec) 
{
  SOL_LOGI(TAG, "clock_set_timer %u sec", sec);
  clock_timer_saved_sec = clock_get_now_sec();
  clock_timer_diff_sec = sec;
}