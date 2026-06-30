#include <STC8G.H>
#include "intrins.h"

#include "adc.h"
#include "debug.h"

#define ADC_MAX_TRY_CNT 3

void adc_initialize(void)
{
  CDBG("adc init");
  P_SW2 |= 0x80;
  
  
  //设置ADC 内部时序
  // SMPDUTY = 11111：Tduty  = 32个时钟
  // CSHOLD  = 11： Thold = 4个时钟
  // Tconvert = 固定10个时钟（10bits ADC）
  // CSSETUP  = 0: Tsetup = 1个时钟
  ADCTIM = 0x3f; 
  
  P_SW2 &= 0x7f;
  
  // 设置ADC 时钟为系统时钟/2/16,转换结果右对齐
  ADCCFG = 0x2f; 
  
  // ADC_EPWMT = 0, 关闭ADC电源
  ADC_CONTR = 0x00;  
}

/*
多通道切换“残留”：
  通道切换后未丢弃第一帧如果你的程序在循环切换不同的 ADC 通道（如先测 A0，再测 A1）：
  原因分析：由于内部多路开关切换需要时间，且采样电容上残存着上一个通道的电荷，
  若切换通道后立即启动转换，会导致逼近网络紊乱，偶尔会直接解算出 0 码。
  对策：在切换 ADC_CONTR 中的通道选择位后，
  必须丢弃切换后的第一次转换结果，从第二次转换开始作为有效数据。
*/
uint16_t adc_get_val(uint8_t ch)
{
  uint16_t val = 0;
  uint8_t adc_try_cnt = 0;
  ADC_CONTR = (ch & 0xF);
  
  //开启AD电源, 启动第一次AD转换
  ADC_CONTR |= 0x80;
  _nop_();
  _nop_();
  
  do {
    // 启动第AD转换
    ADC_CONTR |= 0x40;
    _nop_();
    _nop_();  
    
    while (!(ADC_CONTR & 0x20)); //查询ADC 完成标志
    ADC_CONTR &= ~0x20; //清完成标志
    val = ADC_RES;
    val <<= 8;
    val |= ADC_RESL;
    CDBG("adc_get_val 0x%04x on ch %bu, try_cnt %bu", val, ch, adc_try_cnt);
  } while(val == 0 && ++adc_try_cnt < ADC_MAX_TRY_CNT);
  
  ADC_CONTR = 0x00; //关闭AD电源
  
  return val;
}