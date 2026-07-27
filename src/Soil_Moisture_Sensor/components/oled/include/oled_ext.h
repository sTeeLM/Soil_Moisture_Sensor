#ifndef SOL_OLED_EXT_H
#define SOL_OLED_EXT_H

#include <stdint.h>
#include <stdbool.h>

void oled_ext_draw_percent(uint8_t x, uint8_t y);
void oled_ext_draw_dig(uint8_t x, uint8_t y, uint8_t d, bool point);
void oled_ext_draw_power(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t percent, bool last_on_off);

#endif // SOL_OLED_EXT_H