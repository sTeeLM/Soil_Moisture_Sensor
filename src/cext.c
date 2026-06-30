#include "cext.h"
#include <string.h>

// 64位整数加减乘除
// 核心函数：对 64 位数求补码（取反加 1，用于正负号转换）
uint64_t neg64(uint64_t n) 
{
    uint64_t res;
    res.low = ~n.low;
    res.high = ~n.high;
    res.low++;
    if (res.low == 0) { // 低位溢出，向高位进位
        res.high++;
    }
    return res;
}

// 64位加法
uint64_t add64(uint64_t n1, uint64_t n2) 
{
    uint64_t res;
    res.low = n1.low + n2.low;
    res.high = n1.high + n2.high;
    if (res.low < n1.low) { // 检查低 32 位是否发生溢出进位
        res.high++;
    }
    return res;
}

// 64位减法
uint64_t sub64(uint64_t n1, uint64_t n2) 
{
    uint64_t res;
    res.low = n1.low - n2.low;
    res.high = n1.high - n2.high;
    if (n1.low < n2.low) { // 检查低 32 位是否发生借位
        res.high--;
    }
    return res;
}

// 无符号 64位乘法（大数拆解法）
uint64_t mul64_u(uint64_t n1, uint64_t n2) 
{
    uint64_t res;
    uint32_t h1, l1, h2, l2;
    uint32_t m0, m1, m2, m3;

    // 进一步拆分为 16 位片段以防 32 位乘法在 C51 中自身溢出
    l1 = n1.low & 0xFFFF; h1 = n1.low >> 16;
    l2 = n2.low & 0xFFFF; h2 = n2.low >> 16;

    // 基础交叉乘积
    m0 = l1 * l2;
    m1 = h1 * l2 + (m0 >> 16);
    m2 = l1 * h2 + (m1 & 0xFFFF);
    m3 = h1 * h2 + (m1 >> 16) + (m2 >> 16);

    res.low = (m2 << 16) | (m0 & 0xFFFF);
    
    // 计算高 32 位的基础部分加上交叉跨步溢出
    res.high = m3 + (n1.high * n2.low) + (n1.low * n2.high); 
    return res;
}

// 有符号 64位乘法
int64_t mul64_s(int64_t n1, int64_t n2) 
{
    int8_t sign = 0;
    int64_t res;
    
    if (IS_NEGATIVE64(n1)) { n1 = neg64(n1); sign ^= 1; }
    if (IS_NEGATIVE64(n2)) { n2 = neg64(n2); sign ^= 1; }
    
    res = mul64_u(n1, n2); // 绝对值做无符号乘法
    
    if (sign) { res = neg64(res); } // 异号则结果取负
    return res;
}

//  除法（长除法状态机）
uint64_t div64_u(uint64_t dividend, uint64_t divisor, uint64_t *remainder) 
{
    uint64_t quot = {0, 0};
    uint64_t rem = {0, 0};
    int8_t i;

    if (divisor.high == 0 && divisor.low == 0) return quot; // 除 0 保护

    for (i = 63; i >= 0; i--) {
        // 1. 余数寄存器左移 1 位
        rem.high = (rem.high << 1) | (rem.low >> 31);
        rem.low = (rem.low << 1);
        
        // 2. 将被除数的当前位移入余数低位
        if (i >= 32) {
            rem.low |= (dividend.high >> (i - 32)) & 1;
        } else {
            rem.low |= (dividend.low >> i) & 1;
        }

        // 3. 比较余数与除数大小
        if (rem.high > divisor.high || (rem.high == divisor.high && rem.low >= divisor.low)) {
            rem = sub64(rem, divisor); // 余数减去除数
            if (i >= 32) {
                quot.high |= (1UL << (i - 32)); // 商的对应位置 1
            } else {
                quot.low |= (1UL << i);
            }
        }
    }

    if (remainder) *remainder = rem; // 返回余数
    return quot;
}

// 有符号 64位除法
int64_t div64_s(int64_t dividend, int64_t divisor, int64_t *remainder) 
{
    int8_t q_sign = 0, r_sign = 0;
    int64_t quot, rem;

    if (IS_NEGATIVE64(dividend)) { dividend = neg64(dividend); q_sign ^= 1; r_sign = 1; }
    if (IS_NEGATIVE64(divisor)) { divisor = neg64(divisor); q_sign ^= 1; }

    quot = div64_u(dividend, divisor, &rem);

    // C99标准：商的符号由两数异或决定，余数的符号与被除数一致
    if (q_sign) quot = neg64(quot);
    if (r_sign) rem = neg64(rem);

    if (remainder) *remainder = rem;
    return quot;
}


// 移位
uint64_t shift64_right(uint64_t n, uint8_t bits) 
  {
    uint64_t res = {0, 0};
    if (bits >= 64) return res;
    if (bits >= 32) {
        res.low = n.high >> (bits - 32);
    } else if (bits > 0) {
        res.low = (n.low >> bits) | (n.high << (32 - bits));
        res.high = n.high >> bits;
    } else {
        res = n;
    }
    return res;
}

uint64_t shift64_left(uint64_t n, uint8_t bits) 
{
    uint64_t res = {0, 0};
    if (bits >= 64) return res;
    if (bits >= 32) {
        res.high = n.low << (bits - 32);
    } else if (bits > 0) {
        res.high = (n.high << bits) | (n.low >> (32 - bits));
        res.low = n.low << bits;
    } else {
        res = n;
    }
    return res;
}

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

int32_t cext_linear_interpolate(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x)
{
  int64_t dx = {0,0}, dy = {0,0}, run = {0, 0}, tmp = {0, 0};
// 检查两点是否重合（导致斜率无穷大）
    if (x1 == x2) {
        return 0;
    }

    // 两点式直线方程: y = y1 + (x - x1) * (y2 - y1) / (x2 - x1)
    // 使用 int64_t 防止 (x - x1) * (y2 - y1) 乘法溢出
    //dx = (int64_t)x - x1;
    dx.low = x;
    tmp.low = x1;
    dx = sub64(*(uint64_t*)&x, *(uint64_t*)&x1);
    
    //dy = (int64_t)y2 - y1;
    dy.low = y2;
    tmp.low = y1;
    dy = sub64(*(uint64_t*)&y2, *(uint64_t*)&y1);
    
    //run = (int64_t)x2 - x1;
    run = sub64(*(uint64_t*)&x2, *(uint64_t*)&x1);

    // 计算结果并强制转换为 int32_t 返回
    //y = y1 + (dx * dy) / run;
    tmp = mul64_s(dx, dy);
    tmp = div64_s(tmp, run, NULL);
    tmp = add64(*(uint64_t*)&y1, *(uint64_t*)&tmp);
    
    return tmp.low;
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
