#include <STC8G.H>
#include <string.h>
#include "clock.h"
#include "sm.h"
#include "debug.h"
#include "task.h"
#include "power.h"

// 时钟频率 11059200 HZ
static uint32_t jiff;
static uint32_t sec_now;
static uint8_t timer_sec;

static clock_struct_t clk;

void clock_initialize(void)
{
  CDBG("clock init");
  // 输出时钟频率 = (SYSclk)/12/(65536-[RL_TH0, RL_TL0])/2 = 1600HZ
  TR0 = 0;        //定时器禁止工作
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TL0 = 0x40;				//设置定时初始值
	TH0 = 0xF9;				//设置定时初始值
	ET0 = 0;			  //禁止定时器中断
  
  memset(&clk, 0, sizeof(clk));
  
  sec_now = 0;
  jiff    = 0;
  
  TR0 = 1;        //定时器允许工作
  ET0 = 1;        //定时器允许中断
  
}

uint32_t clock_get_hour(void)
{
  return clk.hour;
}

uint8_t clock_get_min(void)
{
  return clk.min;
}

uint8_t clock_get_sec(void)
{
  return clk.sec;
}

uint16_t clock_get_ms625(void)
{
  return clk.ms625;
}

static void clock_isr (void) interrupt 1 using 1
{
  clk.ms625 ++;
  jiff ++;
  if((clk.ms625 % 1600) == 0 ) {
    clk.ms625 = 0;
    ++ clk.sec;
    task_set(EV_1S);
    clk.sec = clk.sec % 60;
    if((clk.sec % 10) == 0) {
       task_set(EV_10S);
    }
    if(clk.sec == 0) {
      ++ clk.min;
      clk.min %=  60;
      if(clk.min == 0) {
        ++ clk.hour;
      }
    } 
  }  
  if(jiff % 400 == 0) {
    if(jiff % 1600 == 0) {
      sec_now ++;
      if(timer_sec) {
        if(!(--timer_sec)) {
          task_set(EV_TIMEO);
        }
      }
    }
  }
  task_set(EV_KEY_SCAN); // 每秒扫描16次
  TF0 = 0;
}

void clock_time_proc(enum task_events ev)
{
  sm_run(ev);
}

uint32_t clock_get_now_sec(void)
{
  return sec_now;
}


uint32_t clock_diff_now_sec(uint32_t sec)
{
  return (uint32_t)(sec_now - sec);
}

uint32_t clock_get_now_jiff(void)
{
  return jiff;
}

uint32_t clock_diff_now_jiff(uint32_t saved_jiff)
{
  return (uint32_t)(jiff - saved_jiff);
}

void clock_set_timer(uint8_t sec)
{
  timer_sec = sec;
}
