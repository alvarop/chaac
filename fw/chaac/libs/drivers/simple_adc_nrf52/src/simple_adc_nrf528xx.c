#ifndef __SIMPLE_ADC__
#define __SIMPLE_ADC__

#include <adc/adc.h>
#include <simple_adc/simple_adc.h>
#include <nrfx_saadc.h>
#include <bsp/bsp.h>

static struct adc_dev *adc;
nrfx_saadc_config_t adc_config = NRFX_SAADC_DEFAULT_CONFIG;

int32_t adc_init_ch(uint8_t ch, nrf_saadc_input_t saadc_in) {
    if (adc == NULL) {
        return -1;
    } else {

        // Channel config pin is ch + 1
        nrf_saadc_channel_config_t cc = \
            NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(saadc_in);
        cc.gain = NRF_SAADC_GAIN1_6;
        cc.reference = NRF_SAADC_REFERENCE_INTERNAL;

        return adc_chan_config(adc, ch, &cc);
    }
}

int32_t simple_adc_init(void) {
    int32_t rval = 0;

    adc = (struct adc_dev *) os_dev_open("adc0", 0, &adc_config);
    assert(adc != NULL);

    adc_init_ch(BATT_ADC_CH, BATT_SAADC);
    adc_init_ch(WX_DIR_ADC_CH, WX_DIR_SAADC);
    adc_init_ch(VSOLAR_ADC_CH, VSOLAR_SAADC);

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
