#ifndef __SIMPLE_ADC__
#define __SIMPLE_ADC__

#include <adc/adc.h>
#include <simple_adc/simple_adc.h>
#include <nrfx_saadc.h>
#include <bsp/bsp.h>
#include <adc_nrf52/adc_nrf52.h>

static struct adc_dev *adc;
struct adc_dev_cfg adc_config = {
    .resolution = (adc_resolution_t)ADC_RESOLUTION_12BIT,
    .oversample=(adc_oversample_t)ADC_OVERSAMPLE_16X,
    .calibrate=false
};

int32_t simple_adc_init(void) {
    int32_t rval = 0;

    adc = (struct adc_dev *) os_dev_open("adc0", 0, &adc_config);
    assert(adc != NULL);

    adc_chan_config(adc, BATT_ADC_CH,  (void *)&(struct adc_chan_cfg){
        .gain = ADC_GAIN1_6,
        .reference = ADC_REFERENCE_INTERNAL,
        .acq_time = ADC_ACQTIME_20US,
        .pin = BATT_SAADC,
        .differential = false});
     
    adc_chan_config(adc, WX_DIR_ADC_CH,  (void *)&(struct adc_chan_cfg){
        .gain = ADC_GAIN1_6,
        .reference = ADC_REFERENCE_INTERNAL,
        .acq_time = ADC_ACQTIME_20US,
        .pin = WX_DIR_SAADC,
        .differential = false});

#ifdef VSOLAR_ADC_CH
    adc_chan_config(adc, VSOLAR_ADC_CH,  (void *)&(struct adc_chan_cfg){
        .gain = ADC_GAIN1_6,
        .reference = ADC_REFERENCE_INTERNAL,
        .acq_time = ADC_ACQTIME_20US,
        .pin = VSOLAR_SAADC,
        .differential = false});
#endif

    return rval;
}

int32_t simple_adc_uninit(void) {
    int32_t rval = 0;

    rval = os_dev_close((struct os_dev *)adc);
    assert(rval == 0);

    return rval;
}

int32_t simple_adc_read_ch(uint8_t ch, int32_t *val_mv) {
    int32_t rval = 0;
    // int result = 0;

    assert(adc != NULL);
    assert(val_mv != NULL);

    rval = adc_chan_read(adc, ch, (int*)val_mv);
    *val_mv = adc_result_mv(adc, ch, *val_mv);

    return rval;
}

#endif /* __SIMPLE_ADC__ */

