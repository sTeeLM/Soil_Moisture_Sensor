#include <STC8G.H>
#include "moisture.h"
#include "debug.h"

void moisture_initialize(void)
{
  CDBG("moisture init");
  
  CCON = 0x00;
  CMOD = 0x08; //PCA 时钟为系统时钟 = 33177600 Hz，禁止中断
  CL = 0x00;
  CH = 0x00;

  /**
  6位PWM
    当EPCnH=0及CCAPnH=00H时，PWM固定输出高
    当EPCnH=1及CCAPnH=FFH时，PWM固定输出低
  
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
}