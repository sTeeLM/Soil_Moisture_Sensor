#include "cext.h"
#include <string.h>

/////////////////////////////////////////////////////

int16_t cext_linear_interpolate_short(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x)
{
  int32_t dx, dy, run;
  int16_t y;
  // 检查两点是否重合（导致斜率无穷大）
  if (x1 == x2) {
    return 0;
  }
    
  // 两点式直线方程: y = y1 + (x - x1) * (y2 - y1) / (x2 - x1)
  // 使用 int32_t 防止 (x - x1) * (y2 - y1) 乘法溢出
  dx = (int32_t)x - x1;
  dy = (int32_t)y2 - y1;
  run = (int32_t)x2 - x1;
  y = y1 + (dx * dy) / run;
  return y;
}

float cext_linear_interpolate_float(float x1, float y1, float x2, float y2, float x)
{
  double dx, dy, run;
  float y;
// 检查两点是否重合（导致斜率无穷大）
    if (x1 == x2) {
        return 0.0f;
    }

    // 两点式直线方程: y = y1 + (x - x1) * (y2 - y1) / (x2 - x1)
    // 使用 double 防止 (x - x1) * (y2 - y1) 乘法溢出
    dx = (double)x - x1;
    dy = (double)y2 - y1;
    run = (double)x2 - x1;

    // 计算结果并强制转换为 int32_t 返回
    y = (float)(y1 + (dx * dy) / run);
    
    return y;
}

uint16_t cext_iir_uint16(uint16_t oldv, uint16_t newv, uint8_t coe)
{
  int32_t ret = oldv;
  ret += (int32_t)(((int32_t)newv - (int32_t)oldv) / (float)coe);
  return  (uint16_t) ret;
}

float cext_iir_float(float oldv, float newv, uint8_t coe)
{
  float ret = oldv;
  ret += (float)((newv - oldv) / coe);
  return ret;
}
