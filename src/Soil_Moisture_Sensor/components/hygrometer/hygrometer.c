#include "hygrometer.h"
#include "adc_wrapper.h"
#include "logger.h"
#include "delay.h"
#include "cext.h"
#include "gpio_wrapper.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

#define HYGROMETER_MAX_VAL 1400
#define HYGROMETER_MIN_VAL 3200

static const char * TAG = "HYGRO";
static uint16_t IRAM_ATTR hygrometer_sensor_data;
static bool hygrometer_enabled;

static uint16_t hygrometer_get_percent_internal(void);
void hygrometer_init(void)
{
  ledc_timer_config_t ledc_timer = {
      .speed_mode       = LEDC_LOW_SPEED_MODE, // ESP32-C3 仅支持低速模式
      .timer_num        = LEDC_TIMER_0,
      .duty_resolution  = LEDC_TIMER_12_BIT,   // 13位分辨率 (占空比 0 - 4095)
      .freq_hz          = 10000,                // 频率 1 mHz
      .clk_cfg          = LEDC_AUTO_CLK
  };  
  ledc_channel_config_t ledc_channel = {
      .speed_mode     = LEDC_LOW_SPEED_MODE,
      .channel        = LEDC_CHANNEL_0,
      .timer_sel      = LEDC_TIMER_0,
      .intr_type      = LEDC_INTR_DISABLE,
      .gpio_num       = GPIO_PIN_HUMIDITY_PWM, // 输出引脚 GPIO
      .duty           = 0,                     // 初始占空比为 0
      .hpoint         = 0
  }; 
  
  SOL_LOGI(TAG, "init");
  // 1. 配置 LEDC 定时器
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

  // 2. 配置 LEDC 通道
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

  ESP_ERROR_CHECK(ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0));

  hygrometer_enabled = false;

  if(!hygrometer_sensor_data) {
    hygrometer_sensor_data = hygrometer_get_percent_internal();
  }
}

void hygrometer_enable(bool enable)
{
  if(enable && !hygrometer_enabled) {
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 2048));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));

  } else if (!enable && hygrometer_enabled){
    ESP_ERROR_CHECK(ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0));
  }

  hygrometer_enabled = enable;
}

static uint16_t hygrometer_get_percent_internal(void)
{
  int32_t adc_value = 0;
  int32_t voltage = 0;
  int32_t percent = 0;

  hygrometer_enable(true);
  delay_ms(100);
  adc_value = adc_wrapper_get_value(1);
  hygrometer_enable(false);

  voltage = adc_value * 3300 / 4095;
  percent = cext_linear_interpolate(HYGROMETER_MIN_VAL, 0, HYGROMETER_MAX_VAL, 1000, voltage);
  percent = cext_limit(percent, 0, 1000);
  SOL_LOGD(TAG, "hygrometer_get_percent_internal adc %d voltage %d percent %0.1f", adc_value, voltage, percent/10.0f);
  return (uint16_t)percent;
}

uint16_t hygrometer_get_percent(void)
{
  uint16_t new_val = 0;
  new_val = hygrometer_get_percent_internal();
  // 使用IIR滤波器平滑电压值
  hygrometer_sensor_data = cext_iir_uint16(hygrometer_sensor_data, new_val, 4); // 4为滤波系数，可根据需要调整
  return hygrometer_sensor_data;
}