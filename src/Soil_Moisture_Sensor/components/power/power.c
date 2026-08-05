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

// 电压读数，mV
static uint16_t RTC_DATA_ATTR power_sensor_data;
// 电源唤醒原因
static power_wakeup_reason_t power_wakeup_reason;
// 电源状态
static power_state_t power_state;
// 是否从standby恢复
static bool RTC_DATA_ATTR power_recover;

#define POWER_VOLTAGE_FIX_OFFSITE (0.26f) // 校准电压偏移量，单位V
#define POWER_FP_THRES 4200 //满电电压
#define POWER_MP_THRES 4000 //中电电压
#define POWER_LP_THRES 3600 //低电电压

#define POWER_IIR_COEFF 16 // IIR滤波系数


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

bool power_is_battery_low(void)
{
  return power_sensor_data <= POWER_MP_THRES;
}

static uint16_t power_get_battery_voltage_mv_internal(void);

// 注意：不要初始化power_state和power_recover
void power_init(void)
{

  SOL_LOGI(TAG, "init");

  // 初始化中断
  ESP_ERROR_CHECK(gpio_intr_enable(GPIO_PIN_POWER_ADP_ON));
  ESP_ERROR_CHECK(gpio_set_intr_type(GPIO_PIN_POWER_ADP_ON, GPIO_INTR_ANYEDGE));
  ESP_ERROR_CHECK(gpio_isr_handler_add(GPIO_PIN_POWER_ADP_ON, power_isr_handler_adp_on, NULL));

  if(!power_sensor_data) {
    power_sensor_data = POWER_FP_THRES;
  }

  power_state = POWER_STATE_MP;
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
  delay_ms(100); // 等待电压稳定
  adc_value = adc_wrapper_get_value(0);
  gpio_wrapper_set_level(GPIO_PIN_POWER_BATTERY_EN, 0);
  // 根据实际的分压电路计算电池电压
  // 假设分压比为2:1，ADC参考电压为3.3V，ADC分辨率为12位
  voltage = ((float)adc_value / 4095.0) * 3.3 * 2 - POWER_VOLTAGE_FIX_OFFSITE; // 分压比为2:1
  SOL_LOGD(TAG, "power_get_battery_voltage_mv_internal: adc_value=%d, voltage=%.3fV", adc_value, voltage);
  return (uint16_t)(voltage * 1000); // 转换为毫伏
}

static void power_update_battery_voltage_mv(void)
{
  uint16_t new_val= power_get_battery_voltage_mv_internal();
  // 使用IIR滤波器平滑电压值
  power_sensor_data = cext_iir_uint16(power_sensor_data, new_val, POWER_IIR_COEFF); // 4为滤波系数，可根据需要调整
}

uint16_t power_get_battery_voltage_mv(void)
{
  return power_sensor_data;
}

uint16_t power_get_battery_voltage_percent(void)
{
  uint16_t voltage = power_get_battery_voltage_mv();
  if(voltage >= POWER_FP_THRES) {
    return 1000; // 100%
  } else if (voltage <= POWER_LP_THRES) {
    return 0; // 0%
  } else {
    // 线性映射电压到百分比
    return (uint16_t)(cext_linear_interpolate(POWER_LP_THRES, 0, POWER_FP_THRES, 1000, voltage));
  }
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
  SOL_LOGI(TAG, "power_enter_standby %u sec", sec);
  power_recover = true;
  // 配置定时器唤醒(单位是：微秒)
  ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(sec * 1000000));

  // 配置 GPIO 外部中断唤醒（只能使用 RTC 引脚：GPIO 0~5）
  // 按下按键或者插上USB都会唤醒
  ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup((1ULL << GPIO_PIN_KEY) | (1ULL << GPIO_PIN_POWER_ADP_ON) 
  //ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup((1ULL << GPIO_PIN_KEY)
  , ESP_GPIO_WAKEUP_GPIO_HIGH));

  // 休眠期间，电源必须有电
  gpio_hold_en(GPIO_PIN_POWER_EN);

  esp_deep_sleep_start();
}

bool power_is_recover_from_standby(void)
{
  SOL_LOGI(TAG, "power_recover_from_standby: %d", power_recover);
  return power_recover;
}

void power_probe(void)
{
  uint16_t power_vol = 0, power_percent = 0;
  power_update_battery_voltage_mv();
  power_vol = power_get_battery_voltage_mv();
  power_percent = power_get_battery_voltage_percent();
  SOL_LOGD(TAG, "power_probe: voltage %dmV, percentage %.2f%%", power_vol, ((float)power_percent)/10.0);
  if((power_vol > POWER_MP_THRES) && (power_state == POWER_STATE_LP)) {
    SOL_LOGI(TAG, "power_probe: recover from low power %dmV", power_vol);
    task_set(EV_MP);
    power_state = POWER_STATE_MP;
  } else if ((power_vol < POWER_LP_THRES) && (power_state == POWER_STATE_MP)) {
    SOL_LOGW(TAG, "power_probe: low power %dmV", power_vol);
    task_set(EV_LP);
    power_state = POWER_STATE_LP;
  }
}