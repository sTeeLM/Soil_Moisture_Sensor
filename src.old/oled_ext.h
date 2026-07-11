#ifndef __SOL_OLED_EXT_H__
#define __SOL_OLED_EXT_H__

#include <stdint.h>
#include <stdbool.h>

// 在特定位置画电量标记，last_on_off表示最后一格画不画（用来实现最后一格闪烁）
void oled_draw_power(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t percent, bool last_on_off);
void oled_draw_dig(uint8_t x, uint8_t y, uint8_t d, bool point);
#endif