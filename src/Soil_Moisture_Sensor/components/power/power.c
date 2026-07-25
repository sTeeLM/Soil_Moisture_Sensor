#include "power.h"
#include "logger.h"
#include "sm.h"
#include "task.h"
#include "cext.h"
#include "delay.h"
#include "gpio_wrapper.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

static const char * TAG = "POWER";

static adc_oneshot_unit_handle_t adc_handle;
static uint16_t power_sensor_data;

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

  adc_oneshot_unit_init_cfg_t init_config = {
    .unit_id  = ADC_UNIT_1,
    .ulp_mode = ADC_ULP_MODE_DISABLE,
  };    
  adc_oneshot_chan_cfg_t config = {
    .bitwidth = ADC_BITWIDTH_DEFAULT,
    .atten = ADC_ATTEN_DB_12,
  };  
  
  SOL_LOGI(TAG, "init");

  // 初始化中断
  ESP_ERROR_CHECK(gpio_intr_enable(GPIO_PIN_POWER_ADP_ON));
  ESP_ERROR_CHECK(gpio_set_intr_type(GPIO_PIN_POWER_ADP_ON, GPIO_INTR_ANYEDGE));
  ESP_ERROR_CHECK(gpio_isr_handler_add(GPIO_PIN_POWER_ADP_ON, power_isr_handler_adp_on, NULL));

  ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle)); 
  // ADC_CHANNEL_0 = GPIO1
  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle,  ADC_CHANNEL_0, &config));   
 
  power_sensor_data = power_get_battery_voltage_mv_internal();
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
  int adc_value = 0;
  gpio_wrapper_set_level(GPIO_PIN_POWER_BATTERY_EN, 1);
  delay_ms(10); // 等待电压稳定
  ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_CHANNEL_0, &adc_value));
  gpio_wrapper_set_level(GPIO_PIN_POWER_BATTERY_EN, 0);
  // 根据实际的分压电路计算电池电压
  // 假设分压比为2:1，ADC参考电压为3.3V，ADC分辨率为12位
  float voltage = (adc_value / 4095.0) * 3.3 * 2; // 分压比为2:1
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
  SOL_LOGI(TAG, "power_dump: ADP_ON=%d STDBY=%d SOL=%d Voltage=%dmV", 
    power_is_adapter_connected(),
    power_is_adapter_standby(),
    power_is_sol_on(),
    power_get_battery_voltage_mv());
}
