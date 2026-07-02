#include <stdio.h>
#include <stdarg.h>
#include "debug.h"
#include "clock.h"

static bit debug_is_on;
  
void debug_initialize(void)
{
  debug_is_on = 0;
}

void debug_onoff(bool enable)
{
  debug_is_on = enable;
}

void debug_printf(const char * fmt, ...)
{ 
  va_list arg_ptr;
  uint32_t jiff = clock_get_now_jiff();
  if(debug_is_on) {
    printf("[%ld:%02bd:%02bd.%02bd] ", clock_get_hour(), clock_get_min(), clock_get_sec(), clock_get_ms625());
    va_start (arg_ptr, fmt); /* format string */
    vprintf (fmt, arg_ptr);
    va_end (arg_ptr);
    printf("\n");
  }
}
