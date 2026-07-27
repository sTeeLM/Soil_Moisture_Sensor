#include "adc_wrapper.h"
#include "logger.h"
#include "esp_adc/adc_oneshot.h"

static const char * TAG = "ADC";
static adc_oneshot_unit_handle_t adc_handle;

void adc_wrapper_init(void)
{
  adc_oneshot_unit_init_cfg_t init_config = {
    .unit_id  = ADC_UNIT_1,
    .ulp_mode = ADC_ULP_MODE_DISABLE,
  };    
  adc_oneshot_chan_cfg_t config = {
    .bitwidth = ADC_BITWIDTH_DEFAULT,
    .atten = ADC_ATTEN_DB_12,
  };  

  SOL_LOGD(TAG, "init");

  // 设置ADC
  ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle)); 
  // ADC_CHANNEL_0 = GPIO0
  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle,  ADC_CHANNEL_0, &config));  
  // ADC_CHANNEL_1 = GPIO1
  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle,  ADC_CHANNEL_1, &config));  
}

int32_t adc_wrapper_get_value(uint8_t channel)
{
  int adc_value = 0;
  ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, (adc_channel_t)(channel), &adc_value));
  return (int32_t)adc_value;
}