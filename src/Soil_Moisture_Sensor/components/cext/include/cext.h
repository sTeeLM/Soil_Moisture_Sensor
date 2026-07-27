#ifndef SOL_CEXT_H
#define SOL_CEXT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// IIR 滤波
uint16_t cext_iir_uint16(uint16_t oldv, uint16_t newv, uint8_t coe);
float cext_iir_float(float oldv, float newv, uint8_t coe);

// 线性拟合
int32_t cext_linear_interpolate(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x);
float cext_linear_interpolate_float(float x1, float y1, float x2, float y2, float x);

/* 无符号时钟代数循环加 N 宏 */
#define cext_ring_add(v, n, minv, maxv) (minv + ((v - minv + n) % (maxv - minv + 1))) 

/* 无符号时钟代数循环减 N 宏 */
#define cext_ring_sub(v, n, minv, maxv) (minv + ((v - minv + ((maxv - minv + 1) - (n % (maxv - minv + 1)))) % (maxv - minv + 1)))

/* 整数上下限 */
#define cext_limit(v, minv, maxv) (v < minv ? minv : (v > maxv ? maxv : v))


#endif // SOL_CEXT_H