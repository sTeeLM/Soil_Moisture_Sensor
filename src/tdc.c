#include <STC8G.H>

#include "tdc.h"
#include "gpio.h"
#include "debug.h"
#include "delay.h"
#include "task.h"
#include "clock.h"
#include "power.h"

void tdc_initialize(void)
{
  CDBG("tdc init");
}

void tdc_trigger(tdc_type_t type)
{
  switch(type){
    case TDC_TYPE_1S:
      clock_set_timer(1);
      break; 
    case TDC_TYPE_5S:
      clock_set_timer(5);
      break;
    case TDC_TYPE_15S:
      clock_set_timer(15);
      break;
    case TDC_TYPE_NONE:
      return;
    break;
    default:
      CDBG("tdc_trigger unknown type %bu",type); 
      return;
  }
}

void tdc_trigger_sleep(tdc_type_t type)
{
  
  switch(type){
    case TDC_TYPE_1S:
      WKTCL = 0x00;  
      WKTCH = 0x80;
    break;
    case TDC_TYPE_5S:
      WKTCL = 0x00;  
      WKTCH = 0xA8;
    break;
    case TDC_TYPE_15S:
      WKTCL = 0xFE;  
      WKTCH = 0xFF;
    break;
    case TDC_TYPE_NONE:
      return;
    break;
    default:
      CDBG("tdc_trigger unknown type %bu",type); 
      return;
  }
  if(!power_sleep()) { // 如果不是按键和插电唤醒，设置EV_TIMEO
    CDBG("tdc_trigger_sleep: wake up and send EV_TIMEO");
    task_set(EV_TIMEO);
  }
}
