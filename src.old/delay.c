#include <intrins.h>
#include "delay.h"
#include "cext.h"


// 时钟频率33177600
static void internal_delay_10us(void) 
{
  uint8_t data i;

	_nop_();
	_nop_();
	i = 80;
	while (--i);
}

static void internal_delay_ms(void)
{
	uint8_t data i, j;

	_nop_();
	_nop_();
	i = 33;
	j = 66;
	do
	{
		while (--j);
	} while (--i);
}

void delay_10us(uint32_t t)
{
  while(t --) {
    internal_delay_10us();
  }
}

void delay_ms(uint32_t t) 
{     
  while(t--) {      
    internal_delay_ms();
  } 
}
