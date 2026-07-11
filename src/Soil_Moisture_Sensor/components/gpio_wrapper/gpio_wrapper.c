#include "gpio_wrapper.h"
#include "hal/gpio_types.h"
#include "logger.h"
#include "driver/gpio.h"

static const char * TAG = "GPIO";

void gpio_wrapper_init(bool keep_power_en)
{
  gpio_config_t io_power_conf = {};
  gpio_config_t io_i2c_conf = {};
  gpio_config_t io_oled_conf = {};
  gpio_config_t io_key_conf = {};  
  gpio_config_t io_humidity_conf = {}; 

  SOL_LOGI(TAG, "init");
  SOL_LOGI(TAG, "before gpio_dump_io_configuration:");
  gpio_dump_io_configuration(stdout, SOC_GPIO_VALID_GPIO_MASK);
  esp_intr_dump(NULL);

  //// 设置power相关GPIO
  //disable interrupt
  io_power_conf.intr_type = GPIO_INTR_DISABLE;
  //set as output mode
  io_power_conf.mode = GPIO_MODE_INPUT;
  //bit mask of the pins that you want to set,e.g.GPIO18/19
  io_power_conf.pin_bit_mask = 
    (1ULL << GPIO_PIN_POWER_BATTERY_ADC)  | 
    (1ULL << GPIO_PIN_POWER_ADP_ON) |
    (1ULL << GPIO_PIN_POWER_STDBY);
  //disable pull-down mode
  io_power_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  //disable pull-up mode
  io_power_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  //configure GPIO with the given settings
  ESP_ERROR_CHECK(gpio_config(&io_power_conf));


  gpio_wrapper_set_level(GPIO_PIN_POWER_BATTERY_EN, 0);
  io_power_conf.intr_type = GPIO_INTR_DISABLE;
  io_power_conf.mode = GPIO_MODE_OUTPUT;
  io_power_conf.pin_bit_mask = 
    (1ULL << GPIO_PIN_POWER_BATTERY_EN);
  io_power_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_power_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  ESP_ERROR_CHECK(gpio_config(&io_power_conf));

  if(!keep_power_en) {
    gpio_wrapper_set_level(GPIO_PIN_POWER_EN, 0);
    io_power_conf.intr_type = GPIO_INTR_DISABLE;
    io_power_conf.mode = GPIO_MODE_OUTPUT;
    io_power_conf.pin_bit_mask = (1ULL << GPIO_PIN_POWER_EN);
    io_power_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_power_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&io_power_conf));
  }

  //// 设置i2c相关GPIO
  io_i2c_conf.intr_type = GPIO_INTR_DISABLE;
  io_i2c_conf.mode = GPIO_MODE_INPUT_OUTPUT_OD;
  io_i2c_conf.pin_bit_mask = 
    (1ULL << GPIO_PIN_I2C_SCL) |
    (1ULL << GPIO_PIN_I2C_SDA);
  io_i2c_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_i2c_conf.pull_up_en = GPIO_PULLDOWN_DISABLE;
  ESP_ERROR_CHECK(gpio_config(&io_i2c_conf));
 
  //// OLED电源
  gpio_wrapper_set_level(GPIO_PIN_OLED_EN, 0);
  io_oled_conf.intr_type = GPIO_INTR_DISABLE;
  io_oled_conf.mode = GPIO_MODE_OUTPUT;
  io_oled_conf.pin_bit_mask = (1ULL << GPIO_PIN_OLED_EN);
  io_oled_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_oled_conf.pull_up_en = GPIO_PULLDOWN_DISABLE;
  ESP_ERROR_CHECK(gpio_config(&io_oled_conf));
  
  ////Key
  io_key_conf.intr_type = GPIO_INTR_DISABLE;
  io_key_conf.mode = GPIO_MODE_INPUT;
  io_key_conf.pin_bit_mask = (1ULL << GPIO_PIN_KEY);
  io_key_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_key_conf.pull_up_en = GPIO_PULLDOWN_DISABLE;
  ESP_ERROR_CHECK(gpio_config(&io_key_conf));

  ////电容测量（湿度）
  gpio_wrapper_set_level(GPIO_PIN_HUMIDITY_PWM, 0);
  io_humidity_conf.intr_type = GPIO_INTR_DISABLE;
  io_humidity_conf.mode = GPIO_MODE_OUTPUT;
  io_humidity_conf.pin_bit_mask = (1ULL << GPIO_PIN_HUMIDITY_PWM);
  io_humidity_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_humidity_conf.pull_up_en = GPIO_PULLDOWN_DISABLE;
  ESP_ERROR_CHECK(gpio_config(&io_humidity_conf));

  io_humidity_conf.intr_type = GPIO_INTR_DISABLE;
  io_humidity_conf.mode = GPIO_MODE_INPUT;
  io_humidity_conf.pin_bit_mask = (1ULL << GPIO_PIN_HUMIDITY_ADC);
  io_humidity_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_humidity_conf.pull_up_en = GPIO_PULLDOWN_DISABLE;
  ESP_ERROR_CHECK(gpio_config(&io_humidity_conf));  


  gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);

  SOL_LOGI(TAG, "after gpio_dump_io_configuration:");
  gpio_dump_io_configuration(stdout, SOC_GPIO_VALID_GPIO_MASK);

}


void gpio_wrapper_set_level(uint32_t gpio_num, uint8_t level)
{
  ESP_ERROR_CHECK(gpio_set_level((gpio_num_t)gpio_num, level));
}

uint8_t gpio_wrapper_get_level(uint32_t gpio_num)
{
  uint8_t ret = 0;
  int val = gpio_get_level((gpio_num_t)gpio_num);
  if(val < 0) {
    SOL_EARLY_LOGE(TAG, "gpio_get_level failed for gpio %u", gpio_num);
  } else {
    ret = (uint8_t)val;
  }
  return ret;
}