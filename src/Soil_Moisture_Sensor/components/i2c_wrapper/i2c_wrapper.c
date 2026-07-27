#include "i2c_wrapper.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "logger.h"
#include "gpio_wrapper.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/gpio_types.h"
#include "driver/gpio.h"
#include "endian.h"
#include "delay.h"

#define I2C_MASTER_TIMEOUT_MS 1000

static const char * TAG = "I2C";

static i2c_master_bus_handle_t i2c_bus_handle;

void i2c_wrapper_init(void)
{
  SOL_LOGI(TAG, "init");
  i2c_bus_handle = NULL;
}

void i2c_wrapper_add_dev(uint16_t addr, uint32_t clk_speed_hz, i2c_wrapper_dev_handle_t * dev_handle)
{
  i2c_device_config_t dev_config = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = addr,
    .scl_speed_hz = clk_speed_hz,
  };
  ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &dev_config, &(dev_handle->real_handle)));
} 

void i2c_wrapper_del_dev(i2c_wrapper_dev_handle_t * dev_handle)
{
  ESP_ERROR_CHECK(i2c_master_bus_rm_device(dev_handle->real_handle));
}

void i2c_wrapper_write(
  i2c_wrapper_dev_handle_t * dev_handle, 
  uint16_t reg_addr, 
  i2c_wrapper_addr_mode_t reg_addr_mode,
  const uint8_t * data, 
  size_t data_len)
{
  uint8_t  reg_addr_buf8;
  uint16_t reg_addr_buf16;
  i2c_master_transmit_multi_buffer_info_t i2c_buffer[2] = {};

  if (reg_addr_mode == I2C_ADDR_MODE_16BIT) {
    reg_addr_buf16 = htobe16(reg_addr); // 保证先发送低字节再发送高字节
    i2c_buffer[0].write_buffer = (const uint8_t *)&reg_addr_buf16;
    i2c_buffer[0].buffer_size = 2;
  } else if (reg_addr_mode == I2C_ADDR_MODE_8BIT) {
    reg_addr_buf8 = (uint8_t)(reg_addr & 0xFF);
    i2c_buffer[0].write_buffer = (const uint8_t *)&reg_addr_buf8;
    i2c_buffer[0].buffer_size = 1;
  } else {
    SOL_LOGE(TAG, "invalid reg_addr_mode");
    return;
  }

  i2c_buffer[1].write_buffer = data;
  i2c_buffer[1].buffer_size = data_len;

  if(i2c_master_multi_buffer_transmit(dev_handle->real_handle, i2c_buffer, 2, I2C_MASTER_TIMEOUT_MS/portTICK_PERIOD_MS) != ESP_OK) {
    SOL_LOGW(TAG, "i2c_master_multi_buffer_transmit failed, retry");
    delay_ms(10);
    i2c_wrapper_bus_reset();
    ESP_ERROR_CHECK(i2c_master_multi_buffer_transmit(dev_handle->real_handle, i2c_buffer, 2, I2C_MASTER_TIMEOUT_MS/portTICK_PERIOD_MS));
  }
}


void i2c_wrapper_read(
  i2c_wrapper_dev_handle_t * dev_handle, 
  uint16_t reg_addr, 
  i2c_wrapper_addr_mode_t reg_addr_mode,
  uint8_t * data, 
  size_t data_len)
{
  uint8_t  reg_addr_buf8;
  uint16_t reg_addr_buf16;
  if (reg_addr_mode == I2C_ADDR_MODE_16BIT) {
    reg_addr_buf16 = htobe16(reg_addr); // 保证先发送低字节再发送高字节
    ESP_ERROR_CHECK(i2c_master_transmit_receive(dev_handle->real_handle,
     (uint8_t *)&reg_addr_buf16, 2, data, data_len, I2C_MASTER_TIMEOUT_MS/portTICK_PERIOD_MS));
  } else if (reg_addr_mode == I2C_ADDR_MODE_8BIT) {
    reg_addr_buf8 = (uint8_t)(reg_addr & 0xFF);
    ESP_ERROR_CHECK(i2c_master_transmit_receive(dev_handle->real_handle,
     (uint8_t *)&reg_addr_buf8, 1, data, data_len, I2C_MASTER_TIMEOUT_MS/portTICK_PERIOD_MS));
  } else {
    SOL_LOGE(TAG, "invalid reg_addr_mode");
    return;
  }
  
}

bool i2c_wrapper_raw_read(
  i2c_wrapper_dev_handle_t * dev_handle, 
  uint8_t * data, 
  size_t data_len)
{
  return i2c_master_receive(dev_handle->real_handle, data, 
    data_len, I2C_MASTER_TIMEOUT_MS/portTICK_PERIOD_MS) == ESP_OK;
}

bool i2c_wrapper_raw_write(
  i2c_wrapper_dev_handle_t * dev_handle, 
  const uint8_t * data, 
  size_t data_len)
{
  return i2c_master_transmit(dev_handle->real_handle, data, 
    data_len, I2C_MASTER_TIMEOUT_MS/portTICK_PERIOD_MS) == ESP_OK;
}

void i2c_wrapper_bus_reset(void)
{
  SOL_LOGW(TAG, "i2c_wrapper_bus_reset!");
  i2c_master_bus_reset(i2c_bus_handle);
}

void i2c_wrapper_bus_enable(bool enable)
{
  gpio_config_t io_i2c_conf = {};
  i2c_master_bus_config_t i2c_mst_config = {
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .i2c_port   = I2C_NUM_0,
      .scl_io_num = GPIO_PIN_I2C_SCL,
      .sda_io_num = GPIO_PIN_I2C_SDA,
      .glitch_ignore_cnt = 7,
      .flags.enable_internal_pullup = true
  };  

  SOL_LOGD(TAG, "i2c_wrapper_bus_enable(%d)", enable);
  if(enable) {
    gpio_wrapper_set_level(GPIO_PIN_I2C_EN, 0);
    delay_ms(10); // 等待电源稳定
    //// 设置i2c相关GPIO
    io_i2c_conf.intr_type = GPIO_INTR_DISABLE;
    io_i2c_conf.mode = GPIO_MODE_INPUT_OUTPUT_OD;
    io_i2c_conf.pin_bit_mask = 
      (1ULL << GPIO_PIN_I2C_SCL) |
      (1ULL << GPIO_PIN_I2C_SDA);
    io_i2c_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_i2c_conf.pull_up_en = GPIO_PULLDOWN_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&io_i2c_conf));
    delay_ms(10); // 等待I2C稳定
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &i2c_bus_handle));
    i2c_wrapper_bus_reset();
  } else {
    // 关闭I2C总线
    if(i2c_bus_handle) {
      ESP_ERROR_CHECK(i2c_del_master_bus(i2c_bus_handle));
      i2c_bus_handle = NULL;
    }
    // 设置i2c相关GPIO为DISABLE模式，避免漏电
    io_i2c_conf.intr_type = GPIO_INTR_DISABLE;
    io_i2c_conf.mode = GPIO_MODE_DISABLE;
    io_i2c_conf.pin_bit_mask = 
      (1ULL << GPIO_PIN_I2C_SCL) |
      (1ULL << GPIO_PIN_I2C_SDA);
    io_i2c_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_i2c_conf.pull_up_en = GPIO_PULLDOWN_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&io_i2c_conf));
    gpio_wrapper_set_level(GPIO_PIN_I2C_EN, 1);
  }
}