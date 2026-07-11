#ifndef __SOL_MOISTURE_H__
#define __SOL_MOISTURE_H__

#include <stdbool.h>
#include <stdint.h>

void moisture_initialize(void);
void moisture_enable(bool enable);
uint16_t moisture_get_data(void);
void moisture_probe(void);
#endif // __SOL_MOISTURE_H__