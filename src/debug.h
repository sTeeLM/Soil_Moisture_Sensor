#ifndef __SOL_DEBUG_H__
#define __SOL_DEBUG_H__

#include <stdbool.h>

void debug_onoff(bool enable);
void debug_initialize(void);


#ifdef __SOL_DEBUG__
void debug_printf(const char * fmt, ...);
#define CDBG debug_printf
#else
#define CDBG
#endif

void debug_printf(const char * fmt, ...);

//#define CDBG debug_printf

#endif
