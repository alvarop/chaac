#ifndef __SIMPLE_ADC__
#define __SIMPLE_ADC__

#include <stm32l4xx_hal_dma.h>
#include <stm32l4xx_hal_adc.h>
#include <adc/adc.h>
#include <simple_adc/simple_adc.h>

static struct adc_dev *adc;

// mynewt adc library takes uint8_t for channel
// stm32l432 hal adc channel #defines are 32-bit
static const uint32_t stm32l432_ch_list[] = {
    ADC_CHANNEL_0,
    ADC_CHANNEL_1,
    ADC_CHANNEL_2,
    ADC_CHANNEL_3,
    ADC_CHANNEL_4,
    ADC_CHANNEL_5,
    ADC_CHANNEL_6,
    ADC_CHANNEL_7,
    ADC_CHANNEL_8,
    ADC_CHANNEL_9,
    ADC_CHANNEL_10,
    ADC_CHANNEL_11,
    ADC_CHANNEL_12,
    ADC_CHANNEL_13,
    ADC_CHANNEL_14,
    ADC_CHANNEL_15,
    ADC_CHANNEL_16,
    ADC_CHANNEL_17,
    ADC_CHANNEL_18,
    ADC_CHANNEL_VREFINT,
    ADC_CHANNEL_TEMPSENSOR,
    ADC_CHANNEL_VBAT,
    };

int32_t simple_adc_init(void) {
    int32_t rval = 0;

    adc = (struct adc_dev *) os_dev_open("adc1", 0, NULL);
    assert(adc != NULL);

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
    int result = 0;

    assert(adc != NULL);
    assert(ch < sizeof(stm32l432_ch_list)/sizeof(uint32_t));
    assert(val_mv != NULL);

    // Takes around 2.7ms to sample with 640.5 cycle sample time and
    // 256 oversampling ratio (measured with saleae)
    ADC_ChannelConfTypeDef adc_ch_config = {
        .Channel = stm32l432_ch_list[ch],
        .Rank = ADC_REGULAR_RANK_1,
        .SamplingTime = ADC_SAMPLETIME_640CYCLES_5,
        .SingleDiff = ADC_SINGLE_ENDED,
        .OffsetNumber = ADC_OFFSET_NONE,
        .Offset = 0,
    };

    do {
        rval = adc_chan_config(adc, ch, &adc_ch_config);
        if(rval) {
            break;
        }

        rval = adc_sample(adc);
        if(rval) {
            break;
        }

        rval = adc_chan_read(adc, 10, &result);
        if(rval) {
            break;
        }

        *val_mv = adc_result_mv(adc, 10, result);
    } while(0);

    return rval;
}

#endif /* __SIMPLE_ADC__ */
