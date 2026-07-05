#include <STC8G.H>
#include "moisture.h"
#include "debug.h"
#include "adc.h"
#include "delay.h"
#include "cext.h"

static uint16_t moisture_saved_data;

static uint16_t moisture_get_data_internal(void);
void moisture_initialize(void)
{
  CDBG("moisture init");
  
  CCON = 0x00;
  CMOD = 0x08; //PCA 时钟为系统时钟 = 33177600 Hz，禁止中断
  CL = 0x00;
  CH = 0x00;

  moisture_saved_data = moisture_get_data_internal();
}

void moisture_enable(bool enable)
{
  if(enable) {
    /**
    占空比计算：
      0%[(40H-value)/40H]
      value =  [EPCnH][CCAPnH], 7 bits
      0x000（占空比100%） -> 0x3F（占空比0%），一共1024个设置
    */  
    CCAPM0 = 0x42; //PCA 模块0 为PWM工作模式，无中断
    PCA_PWM0 = 0x80; //PCA 模块0 输出6位PWM，频率 PCA时钟输入源频率/(64)
    CCAP0L = 0x20; //PWM 占空比为50%[(20H-40H)/40H]
    CCAP0H = 0x20;
    
    CR = 1;
  } else {
    /**
      6位PWM
      当EPCnH=0及CCAPnH=00H时，PWM固定输出高
      当EPCnH=1及CCAPnH=FFH时，PWM固定输出低
    */
    PCA_PWM0 = 0x82;
    CCAP0H = 0xFF;
    CCAP0L = 0xFF;
    CR = 0;
  }
}

#define MOISTURE_0_ADC_VAL 0x398
#define MOISTURE_100_ADC_VAL 0x265

uint16_t moisture_get_data(void)
{
  return moisture_saved_data;
}

static uint16_t moisture_get_data_internal(void)
{
  int16_t moisture_adc_val;
  int16_t moisture_val;
  
  moisture_enable(true);
  delay_ms(1000);
  
  moisture_adc_val = (int16_t) adc_get_val(2);
  
  moisture_enable(false);
  /*
    0x3FF -> 0%
    0x1FF -> 100%
  */
  moisture_val = cext_linear_interpolate_short(MOISTURE_0_ADC_VAL, 0, MOISTURE_100_ADC_VAL, 1000, moisture_adc_val);
  CDBG("moisture_get_data %04X -> %d", moisture_adc_val, moisture_val);
  
  if(moisture_val < 0) moisture_val = 0;
  if(moisture_val > 1000) moisture_val = 1000;
  return moisture_val;
}

void moisture_probe(void )
{
  uint16_t new_data = moisture_get_data_internal();
  moisture_saved_data = cext_iir_uint16(moisture_saved_data, new_data, 32);
  CDBG("moisture_probe %d", moisture_saved_data);
}