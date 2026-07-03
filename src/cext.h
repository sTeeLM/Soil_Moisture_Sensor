#ifndef __SOL_C_EXT_H__
#define __SOL_C_EXT_H__
/* Commons definitions */

#include <stdint.h>
#include <stdbool.h>

#define  HIGH           1
#define  LOW            0

#define  ON             1
#define  OFF            0

#define  ENABLED        1
#define  DISABLED       0

#define  uchar          unsigned char
#define  uint           unsigned int
#define  ulong          unsigned long

#define  forever        while(1)

#define  low_byte(int_num)  *((char*)&int_num + 1)
#define  high_byte(int_num) *(char*)&int_num

#define  NAN            (ulong)0xFFFFFFFF
#define  PINF           (ulong)0x7F800000
#define  MINF           (ulong)0XFF800000

#define  KCR             0x0D
#define  KLF             0x0A
#define  ESC             0x1B

#define  UNUSED_PARAM(x) \
  do {                   \
    x;                   \
  }while(0)

int16_t cext_linear_interpolate_short(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x);
float cext_linear_interpolate_float(float x1, float y1, float x2, float y2, float x);
#endif
