#include <stc8g.h>
#include "system.h"

void system_init(void)
{
  
  P_SW1 &= ~0x30;
  P_SW1 |= 0x10;  // CCP0 = P1.1 -> P3.5
}