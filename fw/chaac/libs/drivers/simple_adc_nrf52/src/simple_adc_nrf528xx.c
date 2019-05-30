#ifndef __SIMPLE_ADC__
#define __SIMPLE_ADC__

#include <adc/adc.h>
#include <simple_adc/simple_adc.h>

// static struct adc_dev *adc;


int32_t simple_adc_init(void) {
    int32_t rval = 0;

    // adc = (struct adc_dev *) os_dev_open("adc1", 0, NULL);
    // assert(adc != NULL);

    return rval;
}

int32_t simple_adc_uninit(void) {
    int32_t rval = 0;

    // rval = os_dev_close((struct os_dev *)adc);
    // assert(rval == 0);

    return rval;
}

int32_t simple_adc_read_ch(uint8_t ch, int32_t *val_mv) {
    int32_t rval = 0;
    // int result = 0;

    // assert(adc != NULL);
    // assert(val_mv != NULL);


    return rval;
}

#endif /* __SIMPLE_ADC__ */
