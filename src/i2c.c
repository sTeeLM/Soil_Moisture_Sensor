#include <STC8G.H>
#include "intrins.h"
#include "i2c.h"
#include "debug.h"
#include "gpio.h"
#include "delay.h"


static bit i2c_busy;

void i2c_initialize(void)
{
  CDBG("i2c init");
  P_SW2 |= 0x80;
  
  // MSSPEED = (F / 400K / 2 - 4) / 2
  // (33177600 / (400 * 1024) / 2 - 4) / 2 = 18.25
  // = 0x12
  
  // 1 1 010010
  I2CCFG = 0xd2; // 使能i2c，400k
  
  I2CMSST = 0x00;  
  
  P_SW2 &= 0x7f;
  
  i2c_busy = 0;
}

static void i2c_isr() interrupt 24 using 2
{
  _push_(P_SW2);
  P_SW2 |= 0x80;
  if (I2CMSST & 0x40)
  {
    I2CMSST &= ~0x40; //清中断标志
    i2c_busy = 0;
  }
  _pop_(P_SW2);
}

static void i2c_start()
{
  P_SW2 |= 0x80;
  i2c_busy = 1;
  I2CMSCR = 0x81; //发送START 命令
  P_SW2 &= 0x7f;
  while (i2c_busy);
}

static void i2c_send_data(uint8_t dat)
{
  P_SW2 |= 0x80;
  I2CTXD = dat; //写数据到数据缓冲区
  i2c_busy = 1;
  I2CMSCR = 0x82; //发送SEND 命令
  P_SW2 &= 0x7f;
  while (i2c_busy);
}

static bit i2c_recv_ack()
{
  bool ret = false;
  P_SW2 |= 0x80;
  i2c_busy = 1;
  I2CMSCR = 0x83; //发送读ACK 命令
  while (i2c_busy);
  // MSACKI
  ret = (I2CMSST & 0x02) != 0;
  P_SW2 &= 0x7f;
  return ret;
}

static uint8_t i2c_recv_data()
{
  uint8_t ret = 0;
  i2c_busy = 1;
  P_SW2 |= 0x80;
  I2CMSCR = 0x84; //发送RECV 命令
  while (i2c_busy);
  P_SW2 &= 0x7f;
  ret = I2CRXD;
  return ret;
}
static void i2c_send_ack()
{
  P_SW2 |= 0x80;
  I2CMSST |= 0x0; //设置ACK 信号
  i2c_busy = 1;
  I2CMSCR = 0x85; //发送ACK 命令
  P_SW2 &= 0x7f;
  while (i2c_busy);
}
static void i2c_send_nack()
{
  P_SW2 |= 0x80;
  I2CMSST = 0x01; //设置NAK 信号
  i2c_busy = 1;
  I2CMSCR = 0x85; //发送ACK 命令
  P_SW2 &= 0x7f;
  while (i2c_busy);
}

static void i2c_stop()
{
  i2c_busy = 1;
  P_SW2 |= 0x80;
  I2CMSCR = 0x86; //发送STOP 命令
  P_SW2 &= 0x7f;
  while (i2c_busy);
}

static void i2c_delay()
{
  uint16_t i;
  for (i = 0; i<3000; i++) {
    _nop_();
    _nop_();
    _nop_();
    _nop_();
  }
}

bool i2c_send_buffer(uint8_t slave_addr, uint16_t reg_addr, i2c_addr_mode_t mode, const uint8_t * buffer, uint8_t buffer_size)
{
  uint8_t addr = slave_addr << 1;
  
  i2c_start();

  i2c_send_data(addr);
  if ( i2c_recv_ack() ) {
    i2c_stop();
    return false;
  }

  i2c_send_data(reg_addr & 0xFF);
  if ( i2c_recv_ack() ) {
    i2c_stop();
    return false;
  }
  
  if(mode == I2C_ADDR_MODE_16BIT) {
    i2c_send_data((reg_addr >> 8) & 0xFF);
    if ( i2c_recv_ack() ) {
      i2c_stop();
      return false;
    }
  }
  
  do {
    i2c_send_data(*buffer++);
    if ( i2c_recv_ack() ) {
      i2c_stop();
      return false;
    }
  } while ( --buffer_size != 0 );

  i2c_stop();
  return true;
}

bool i2c_recv_buffer(uint8_t slave_addr, uint16_t reg_addr, i2c_addr_mode_t mode, uint8_t *buffer, uint8_t buffer_size)
{
  uint8_t addr = slave_addr << 1;

  i2c_start();

  i2c_send_data(addr);
  if ( i2c_recv_ack() ) {
    i2c_stop();
    return false;
  }

  i2c_send_data(reg_addr);
  if ( i2c_recv_ack() ) {
    i2c_stop();
    return false;
  }

  if(mode == I2C_ADDR_MODE_16BIT) {
    i2c_send_data((reg_addr >> 8) & 0xFF);
    if ( i2c_recv_ack() ) {
      i2c_stop();
      return false;
    }
  }  
  
  i2c_start();

  addr |= 0x01;
  i2c_send_data(addr);
  if ( i2c_recv_ack() ) {
    i2c_stop();
    return false;
  }

  for (;;) {
    *buffer++ = i2c_recv_data();
    if ( --buffer_size == 0 ) {
       i2c_send_nack();
       break;
     }
     i2c_send_ack();
  }

  i2c_stop();
  return true;
}



/*----------------------------------
// 软件实现
static void i2c_delay()
{
  uint16_t i;
  for (i = 0; i<3000; i++) {
    _nop_();
    _nop_();
    _nop_();
    _nop_();
  }
}

void i2c_initialize()
{
 CDBG("i2c init");
 GPIO_I2C_SCL = 1;
 i2c_delay();
 GPIO_I2C_SDA = 1;
 i2c_delay();
}


void i2c_start()
{
  GPIO_I2C_SDA = 1;
  i2c_delay();
  GPIO_I2C_SCL = 1;
  i2c_delay();
  GPIO_I2C_SDA = 0;
  i2c_delay();
  GPIO_I2C_SCL = 0;
  i2c_delay();
}

void i2c_stop()
{
  GPIO_I2C_SDA = 0;
  i2c_delay();
  GPIO_I2C_SCL = 1;
  i2c_delay();
  GPIO_I2C_SDA = 1;
  i2c_delay();
  i2c_delay();
  i2c_delay();
  i2c_delay();
	delay_ms(5);
}



void i2c_write(uint8_t dat)
{

  uint8_t t = 8;
  do
  {
     GPIO_I2C_SDA = (bit)(dat & 0x80);
     dat <<= 1;
     GPIO_I2C_SCL = 1;
     i2c_delay();
     GPIO_I2C_SCL = 0;
     i2c_delay();
  } while ( --t != 0 );
}

uint8_t i2c_read()
{
  char dat = 0;
  uint8_t t = 8;
  GPIO_I2C_SDA = 1; //在读取数据之前，要把SDA拉高
  do
  {
     GPIO_I2C_SCL = 1;
     i2c_delay();
     dat <<= 1;
     if ( GPIO_I2C_SDA ) dat |= 0x01;
     GPIO_I2C_SCL = 0;
     i2c_delay();
  } while ( --t != 0 );
  return dat;
}



bit i2c_get_ack()
{
  bit ack;
  GPIO_I2C_SDA = 1;
  i2c_delay();
  GPIO_I2C_SCL = 1;
  i2c_delay();
  ack = GPIO_I2C_SDA;
  GPIO_I2C_SCL = 0;
  i2c_delay();
  return ack;
}

void i2c_put_ack(bit ack)
{
  GPIO_I2C_SDA = ack;
  i2c_delay();
  GPIO_I2C_SCL = 1;
  i2c_delay();
  GPIO_I2C_SCL = 0;
  i2c_delay();
}

bool i2c_send_buffer(uint8_t slave_addr, uint16_t reg_addr, i2c_addr_mode_t mode, const uint8_t * buffer, uint8_t buffer_size)
{
  
 uint8_t addr;
  
 addr = (slave_addr << 1) & 0xFE;

 i2c_start();

 i2c_write(addr);
 if ( i2c_get_ack() )
 {
  i2c_stop();
  return false;
 }

 i2c_write(reg_addr & 0xFF);
 if ( i2c_get_ack() )
 {
  i2c_stop();
  return false;
 }
 
 if(mode == I2C_ADDR_MODE_16BIT) {
   i2c_write((reg_addr >> 8) & 0xFF);
   if ( i2c_get_ack() ) {
     i2c_stop();
     return false;
   }
 } 

 do
 {
  i2c_write(*buffer++);
  if ( i2c_get_ack() )
  {
   i2c_stop();
   return false;
  }
 } while ( --buffer_size != 0 );

 i2c_stop();
 return true;
}


bool i2c_recv_buffer(uint8_t slave_addr, uint16_t reg_addr, i2c_addr_mode_t mode, uint8_t *buffer, uint8_t buffer_size)
{

 uint8_t addr;
  
 addr = (slave_addr << 1) & 0xFE;

 i2c_start();

 i2c_write(addr);
 if ( i2c_get_ack() )
 {
  i2c_stop();
  return false;
 }

 i2c_write(reg_addr & 0xFF);
 if ( i2c_get_ack() )
 {
  i2c_stop();
  return false;
 }
 if(mode == I2C_ADDR_MODE_16BIT) {
   i2c_write((reg_addr >> 8) & 0xFF);
   if ( i2c_get_ack() ) {
     i2c_stop();
     return false;
   }
 } 
 
 i2c_start();

 addr |= 0x01;
 i2c_write(addr);
 if ( i2c_get_ack() )
 {
  i2c_stop();
  return false;
 }

 for (;;)
 {
  *buffer++ = i2c_read();
  if ( --buffer_size == 0 )
  {
   i2c_put_ack(1);
   break;
  }
  i2c_put_ack(0);
 }

 i2c_stop();
 return true;
}

*/
