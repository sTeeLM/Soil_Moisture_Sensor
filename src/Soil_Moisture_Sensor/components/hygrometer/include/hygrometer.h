#ifndef SOL_HOGROMETER_H
#define SOL_HOGROMETER_H

#include <stdint.h>
#include <stdbool.h>

void hygrometer_init(void);
void hygrometer_enable(bool enable);
uint16_t hygrometer_get_percent(void);

#endif // SOL_HOGROMETER_H