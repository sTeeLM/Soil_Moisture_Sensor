#include <stc8g.h>
#include "system.h"

void system_init(void)
{

  //  初始化系统时钟
  P_SW2 = 0x80;
  XOSCCR = 0xc0; //启动外部晶振
  while (!(XOSCCR & 1)); //等待时钟稳定
  CLKDIV = 0x00; //时钟不分频 
  CLKSEL = 0x01;
  P_SW2 = 0x00;
}