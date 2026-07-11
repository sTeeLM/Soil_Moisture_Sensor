#ifndef __SOL_ADC_H__
#define __SOL_ADC_H__

#include <stdint.h>

void adc_initialize(void);
uint16_t adc_get_val(uint8_t ch);

#endif //__SOL_ADC_H__