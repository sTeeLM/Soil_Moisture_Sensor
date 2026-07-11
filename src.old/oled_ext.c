#include "oled_ext.h"
#include "oled.h"
#include "oled_font.h"
// dig w is 20, h is 16

static const char * code oled_dig_array [10] = {
  MINI_FONT_ASCII_0,
  MINI_FONT_ASCII_1,
  MINI_FONT_ASCII_2, 
  MINI_FONT_ASCII_3, 
  MINI_FONT_ASCII_4,
  MINI_FONT_ASCII_5,
  MINI_FONT_ASCII_6,
  MINI_FONT_ASCII_7,
  MINI_FONT_ASCII_8, 
  MINI_FONT_ASCII_9,   
};

void oled_draw_dig(uint8_t x, uint8_t y, uint8_t d, bool point)
{
  oled_draw_bitmap(x, y, 20, 32, oled_dig_array[d], OLED_DRAW_OVERWRITE);
  if(point)
    oled_fill_rect(x+17, y+29, 3, 3, true);
}

void oled_draw_power(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t percent, bool last_on_off)
{
  uint8_t cell_w, cell_h, cell_w_last;
  
  if(w < 4 || h < 4) return;
  
  // 计算每一个格子的宽度，高度
  cell_h = h - 4;
  cell_w = (w - 4)/ 4;
  
  if(cell_h < 1 || cell_w < 1) return;  
  cell_w --;
  
  cell_w_last = (w - 4) % 4 + cell_w + 1;
  
  // 画外框
  oled_fill_rect(x, y, w, h, true);
  oled_fill_rect(x+1, y+1, w-2, h-2, false);
  
  if(percent < 25 ) {
    if(last_on_off)
      oled_fill_rect(x + 2, y + 2, cell_w, cell_h, true);
  } else if(percent >= 25 && percent < 50) {
    oled_fill_rect(x + 2, y + 2, cell_w, cell_h, true);
    if(last_on_off)
      oled_fill_rect(x + 2 + (cell_w + 1), y + 2, cell_w, cell_h, true);
  } else if(percent >= 50 && percent < 75) {
    oled_fill_rect(x + 2, y + 2, cell_w, cell_h, true);
    oled_fill_rect(x + 2 + (cell_w + 1), y + 2, cell_w, cell_h, true);
    if(last_on_off)
      oled_fill_rect(x + 2 + (cell_w + 1) * 2, y + 2, cell_w, cell_h, true);
  } else {
    oled_fill_rect(x + 2, y + 2, cell_w, cell_h, true);
    oled_fill_rect(x + 2 + (cell_w + 1), y + 2, cell_w, cell_h, true);
    oled_fill_rect(x + 2 + (cell_w + 1) * 2, y + 2, cell_w, cell_h, true);
    if(last_on_off)
      oled_fill_rect(x + 2 + (cell_w + 1) * 3, y + 2, cell_w_last, cell_h, true);
  }
  
}  