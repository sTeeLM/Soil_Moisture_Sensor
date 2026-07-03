#ifndef __SOL_I2C_H__
#define __SOL_I2C_H__


#include <stdint.h>
#include <stdbool.h>

typedef enum _i2c_addr_mode_t
{
  I2C_ADDR_MODE_8BIT,
  I2C_ADDR_MODE_16BIT,
}i2c_addr_mode_t;

void i2c_initialize(void);
bool i2c_recv_buffer(uint8_t slave_addr, uint16_t reg_addr, i2c_addr_mode_t mode, uint8_t *buffer, uint8_t buffer_size);
bool i2c_send_buffer(uint8_t slave_addr, uint16_t reg_addr, i2c_addr_mode_t mode, const uint8_t * buffer, uint8_t buffer_size);

#endif //