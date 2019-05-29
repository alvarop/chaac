#ifndef __SIMPLE_ADC__
#define __SIMPLE_ADC__

#include <stdint.h>

int32_t simple_adc_init(void);
int32_t simple_adc_uninit(void);
int32_t simple_adc_read_ch(uint8_t ch, int32_t *val_mv);

#endif /* __SIMPLE_ADC__ */
