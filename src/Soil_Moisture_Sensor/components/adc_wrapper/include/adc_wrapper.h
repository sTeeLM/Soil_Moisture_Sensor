#ifndef SOL_ADC_WRAPPER_H
#define SOL_ADC_WRAPPER_H

#include <stdint.h>

void adc_wrapper_init(void);
int32_t adc_wrapper_get_value(uint8_t channel);

#endif // SOL_ADC_WRAPPER_H