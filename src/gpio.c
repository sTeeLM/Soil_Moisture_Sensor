#include "gpio.h"
#include "debug.h"

void gpio_initialize(void)
{
  CDBG("gpio init");
  
  // 串口0设置为双向
  gpio_set_mode(GPIO_SERIAL_RXD_PORT, GPIO_SERIAL_RXD_BIT, GPIO_MODE_BID);
  gpio_set_mode(GPIO_SERIAL_TXD_PORT, GPIO_SERIAL_TXD_BIT, GPIO_MODE_BID);
  
  // 串口1设置为双向
  gpio_set_mode(GPIO_BT_RXD_PORT, GPIO_BT_RXD_BIT, GPIO_MODE_BID);
  gpio_set_mode(GPIO_BT_TXD_PORT, GPIO_BT_TXD_BIT, GPIO_MODE_BID);
  
  // i2c 设置为双向
  gpio_set_mode(GPIO_I2C_SDA_PORT, GPIO_I2C_SDA_BIT, GPIO_MODE_BID);
  gpio_set_mode(GPIO_I2C_SCL_PORT, GPIO_I2C_SCL_BIT, GPIO_MODE_BID);
  
  // Power Manager
  gpio_set_mode(GPIO_POWER_EN_PORT, GPIO_POWER_EN_BIT, GPIO_MODE_PULL_PUSH_OUT);
  GPIO_POWER_EN = 0;
  gpio_set_mode(GPIO_POWER_CHRG_PORT, GPIO_POWER_CHRG_BIT, GPIO_MODE_IN);
  gpio_set_mode(GPIO_POWER_STDBY_PORT, GPIO_POWER_STDBY_BIT, GPIO_MODE_IN);
  gpio_set_mode(GPIO_POWER_SOL_PORT, GPIO_POWER_SOL_BIT, GPIO_MODE_BID);
  
  // Button
  gpio_set_mode(GPIO_KEY_INT_PORT, GPIO_KEY_INT_BIT, GPIO_MODE_IN);
  
  // LW
  gpio_set_mode(GPIO_LW_INT_PORT, GPIO_LW_INT_BIT, GPIO_MODE_IN);
  
  // Moisture Sensor
  gpio_set_mode(GPIO_MOI_SENSOR_ADC_PORT, GPIO_MOI_SENSOR_ADC_BIT, GPIO_MODE_IN);
  
  // OLED
  gpio_set_mode(GPIO_OLED_EN_PORT, GPIO_OLED_EN_BIT, GPIO_MODE_IN);
}

void gpio_set_mode(uint8_t port, uint8_t bits, gpio_mode_t mode)
{
  if(bits > 8) {
    CDBG("invalid bits %d", bits);
    return;
  }
  switch (port) {
    case GPIO_PORT_P0:
      P0M0 &= ~(1 << bits);
      P0M1 &= ~(1 << bits);
      P0M0 |= (mode & 0x1) << bits;
      P0M1 |= ((mode & 0x2) >> 1) << bits;
      break;
    case GPIO_PORT_P1:
      P1M0 &= ~(1 << bits);
      P1M1 &= ~(1 << bits);
      P1M0 |= (mode & 0x1) << bits;
      P1M1 |= ((mode & 0x2) >> 1) << bits;
      break;
    case GPIO_PORT_P2:
      P2M0 &= ~(1 << bits);
      P2M1 &= ~(1 << bits);
      P2M0 |= (mode & 0x1) << bits;
      P2M1 |= ((mode & 0x2) >> 1) << bits;
      break;
    case GPIO_PORT_P3:
      P3M0 &= ~(1 << bits);
      P3M1 &= ~(1 << bits);
      P3M0 |= (mode & 0x1) << bits;
      P3M1 |= ((mode & 0x2) >> 1) << bits;
      break;
    case GPIO_PORT_P4:
      P4M0 &= ~(1 << bits);
      P4M1 &= ~(1 << bits);
      P4M0 |= (mode & 0x1) << bits;
      P4M1 |= ((mode & 0x2) >> 1) << bits;
      break;
    case GPIO_PORT_P5:
      P5M0 &= ~(1 << bits);
      P5M1 &= ~(1 << bits);
      P5M0 |= (mode & 0x1) << bits;
      P5M1 |= ((mode & 0x2) >> 1) << bits;
      break;
    default:
      CDBG("unknown port %d", port);
  }
}
