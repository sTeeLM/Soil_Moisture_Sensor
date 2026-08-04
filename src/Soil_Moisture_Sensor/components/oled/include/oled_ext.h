#ifndef SOL_OLED_EXT_H
#define SOL_OLED_EXT_H

#include <stdint.h>
#include <stdbool.h>

#include "mini_font.h"

void oled_ext_draw_percent(uint8_t x, uint8_t y);
void oled_ext_draw_dig(uint8_t x, uint8_t y, uint8_t d, bool point);
void oled_ext_draw_power(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t percent, bool last_on_off);

extern const uint8_t OLE_EXT_ICON_ADP[];
extern const uint8_t OLE_EXT_ICON_SUN[];
extern const uint8_t OLE_EXT_ICON_BT[];

void oled_ext_draw_wstring(
  uint8_t x, 
  uint8_t y, 
  const wchar_t * str, 
  mini_font_type_t ascii_font,
  mini_font_type_t wide_char_font,
  oled_draw_type_t type);

// 在 x, y位置写字符串str, str必须是ascii字符
// ascii_font: ascii字符使用的font
// 如果字库中没有对应字符，打印一个方块
void oled_ext_draw_string(
  uint8_t x, 
  uint8_t y, 
  const char * str, 
  mini_font_type_t ascii_font,
  oled_draw_type_t type);

#endif // SOL_OLED_EXT_H