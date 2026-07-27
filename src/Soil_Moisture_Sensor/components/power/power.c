#include "power.h"
#include "logger.h"
#include "sm.h"
#include "task.h"
#include "cext.h"
#include "delay.h"
#include "adc_wrapper.h"
#include "gpio_wrapper.h"
#include "driver/gpio.h"
#include "esp_sleep.h"


static const char * TAG = "POWER";

static uint16_t power_sensor_data;
static power_wakeup_reason_t power_wakeup_reason;

static bool RTC_DATA_ATTR power_recover;

static void IRAM_ATTR power_isr_handler_adp_on (void* param)
{
  if(power_is_adapter_connected()) {
    task_set(EV_ADP_ON);
    SOL_EARLY_LOGI(TAG, "power_isr_handler_adp_on: ADP_ON");
  } else {
    task_set(EV_ADP_OFF);
    SOL_EARLY_LOGI(TAG, "power_isr_handler_adp_on: ADP_OFF");
  }
}

static uint16_t power_get_battery_voltage_mv_internal(void);
void power_init(void)
{

  SOL_LOGI(TAG, "init");

  // 初始化中断
  ESP_ERROR_CHECK(gpio_intr_enable(GPIO_PIN_POWER_ADP_ON));
  ESP_ERROR_CHECK(gpio_set_intr_type(GPIO_PIN_POWER_ADP_ON, GPIO_INTR_ANYEDGE));
  ESP_ERROR_CHECK(gpio_isr_handler_add(GPIO_PIN_POWER_ADP_ON, power_isr_handler_adp_on, NULL));

  power_sensor_data = power_get_battery_voltage_mv_internal();

  power_recover = false;

  power_wakeup_reason = POWER_WAKEUP_NONE;
}

void power_proc(task_event_t ev)
{
  sm_run(ev);
}

bool power_is_adapter_connected(void)
{
  return gpio_wrapper_get_level(GPIO_PIN_POWER_ADP_ON) == 1;
}

bool power_is_adapter_standby(void)
{
  return gpio_wrapper_get_level(GPIO_PIN_POWER_STDBY) == 0;
}

bool power_is_sol_on(void)
{
  return gpio_wrapper_get_level(GPIO_PIN_POWER_SOL) == 1;
}

void power_enable(bool enable)
{
  gpio_wrapper_set_level(GPIO_PIN_POWER_EN, enable ? 1 : 0);
}

static uint16_t power_get_battery_voltage_mv_internal(void)
{
  int32_t adc_value = 0;
  float voltage = 0.0f;
  gpio_wrapper_set_level(GPIO_PIN_POWER_BATTERY_EN, 1);
  delay_ms(10); // 等待电压稳定
  adc_value = adc_wrapper_get_value(0);
  gpio_wrapper_set_level(GPIO_PIN_POWER_BATTERY_EN, 0);
  // 根据实际的分压电路计算电池电压
  // 假设分压比为2:1，ADC参考电压为3.3V，ADC分辨率为12位
  voltage = (adc_value / 4095.0) * 3.3 * 2; // 分压比为2:1
  return (uint16_t)(voltage * 1000); // 转换为毫伏
}

uint16_t power_get_battery_voltage_mv(void)
{
  uint16_t new_val= power_get_battery_voltage_mv_internal();
  // 使用IIR滤波器平滑电压值
  power_sensor_data = cext_iir_uint16(power_sensor_data, new_val, 4); // 4为滤波系数，可根据需要调整
  return power_sensor_data;
}

void power_dump(void)
{
  SOL_LOGD(TAG, "power_dump: ADP_ON=%d STDBY=%d SOL=%d Voltage=%dmV", 
    power_is_adapter_connected(),
    power_is_adapter_standby(),
    power_is_sol_on(),
    power_get_battery_voltage_mv());
}

power_wakeup_reason_t power_get_wakeup_reason(void)
{
  return power_wakeup_reason;
}

void power_check_wakeup_reason(void) 
{
  // 获取唤醒原因
  uint64_t pin_mask = 0;
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_TIMER:
      SOL_LOGD(TAG, "wakeup by timeout");
      power_wakeup_reason = POWER_WAKEUP_TIMEO;
      break;
            
    case ESP_SLEEP_WAKEUP_GPIO:
      pin_mask = esp_sleep_get_gpio_wakeup_status();
      SOL_LOGD(TAG, "wakeup by gpio, pin_mask %lx", pin_mask);
      if(pin_mask & (1 << GPIO_PIN_KEY)) {
        SOL_LOGD(TAG, "wakeup by key");
        power_wakeup_reason = POWER_WAKEUP_KEY;
      } else if (pin_mask & (1 << GPIO_PIN_POWER_ADP_ON)) {
        SOL_LOGD(TAG, "wakeup by adp");
        power_wakeup_reason = POWER_WAKEUP_ADP;
      } else {
        SOL_LOGW(TAG, "unknown pin_mask %lx", pin_mask);
        power_wakeup_reason = POWER_WAKEUP_NONE;
      }
      break;
            
    case ESP_SLEEP_WAKEUP_UNDEFINED:
      default:
      SOL_LOGW(TAG, "unknown wake up reason");
      power_wakeup_reason = POWER_WAKEUP_NONE;
      break;
    }
}

void power_enter_standby(uint32_t sec)
{
  power_recover = true;
  // 配置定时器唤醒(单位是：微秒)
  ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(sec * 1000000));

  // 配置 GPIO 外部中断唤醒（只能使用 RTC 引脚：GPIO 0~5）
  // 按下按键或者插上USB都会唤醒
  //ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup((1ULL << GPIO_PIN_KEY) |(1ULL << GPIO_PIN_POWER_ADP_ON) 
  ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup((1ULL << GPIO_PIN_KEY)
  , ESP_GPIO_WAKEUP_GPIO_HIGH));

  // 休眠期间，电源必须有电
  gpio_hold_en(GPIO_PIN_POWER_EN);

  esp_deep_sleep_start();
}

bool power_recover_from_standby(void)
{
  SOL_LOGI(TAG, "power_recover_from_standby: %d", power_recover);
  return power_recover;
}
