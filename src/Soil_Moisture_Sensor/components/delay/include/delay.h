#ifndef SOL_DELAY_H
#define SOL_DELAY_H

#include <stdint.h>

void delay_init(void);
void delay_ms(uint32_t ms);
void delay_us(uint32_t us);

#endif //SOL_DELAY_H
