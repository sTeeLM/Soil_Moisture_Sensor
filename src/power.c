#include "power.h"
#include "debug.h"
#include "gpio.h"
#include "adc.h"

void power_initialize(void)
{
  CDBG("power init");
}

bool power_adapter_on(void)
{
  if(GPIO_POWER_CHRG == 1 && GPIO_POWER_STDBY == 1)
    return false;
  
  return true;
}

bool power_sol_on(void)
{
  return GPIO_POWER_SOL_PORT != 0;
}

void power_enable(bool enable)
{
  GPIO_POWER_EN = enable ? 1 : 0;
}
uint16_t power_get_vol(void)
{
  int32_t adc_val = (int32_t) adc_get_val(15);
  int32_t vol;
  // 0x3ff-> vol
  // adc_val  -> 1185 mV
  // 求电源电压 vol
  // 电源电压 vol = 1185 * 0x3ff / adc_val;
  vol = (uint16_t)(((int32_t)1185 * 0x3ff) / adc_val);
  return vol;
}

void power_dump(void)
{
  CDBG("CHRG:%bu, STDBY:%bu, SOL:%bu, VOL:%u mV",  
    GPIO_POWER_CHRG ? 0 : 1, 
    GPIO_POWER_STDBY ? 0 : 1, 
    GPIO_POWER_SOL ? 1 : 0,
    power_get_vol());
}