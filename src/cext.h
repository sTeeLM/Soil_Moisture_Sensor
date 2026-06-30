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
  
// 统一的 64 位基础数据结构
typedef struct {
    unsigned long high; // 高 32 位
    unsigned long low;  // 低 32 位
} uint64_t;

// 有符号复用相同结构，最高位(high的第31位)为符号位
typedef uint64_t int64_t;

// 辅助宏：判断有符号数是否为负数
#define IS_NEGATIVE64(n) ((n).high & 0x80000000UL)
uint64_t neg64(uint64_t n);
uint64_t add64(uint64_t n1, uint64_t n2);
uint64_t sub64(uint64_t n1, uint64_t n2);
uint64_t mul64_u(uint64_t n1, uint64_t n2);
int64_t mul64_s(int64_t n1, int64_t n2);
uint64_t div64_u(uint64_t dividend, uint64_t divisor, uint64_t *remainder);
int64_t div64_s(int64_t dividend, int64_t divisor, int64_t *remainder);
uint64_t shift64_right(uint64_t n, uint8_t bits);
uint64_t shift64_left(uint64_t n, uint8_t bits);
int16_t cext_linear_interpolate_short(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x);
int32_t cext_linear_interpolate(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x);
float cext_linear_interpolate_float(float x1, float y1, float x2, float y2, float x);
#endif
