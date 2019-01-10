#ifndef __ADC_STM32L432_H__
#define __ADC_STM32L432_H__

#include <adc/adc.h>
#include <mcu/mcu.h>

#ifdef __cplusplus
extern "C" {
#endif

// #define ADC1_IN0     ?   // VREFINT

#define ADC1_IN5_PIN    MCU_GPIO_PORTA(0)
#define ADC1_IN6_PIN    MCU_GPIO_PORTA(1)
#define ADC1_IN7_PIN    MCU_GPIO_PORTA(2)
#define ADC1_IN8_PIN    MCU_GPIO_PORTA(3)
#define ADC1_IN9_PIN    MCU_GPIO_PORTA(4)
#define ADC1_IN10_PIN   MCU_GPIO_PORTA(5)
#define ADC1_IN11_PIN   MCU_GPIO_PORTA(6)
#define ADC1_IN12_PIN   MCU_GPIO_PORTA(7)

#define ADC1_IN15_PIN   MCU_GPIO_PORTB(0)
#define ADC1_IN16_PIN   MCU_GPIO_PORTB(1)


struct stm32l432_adc_dev_cfg {
    uint8_t sac_chan_count;
    void *sac_chans;
    ADC_HandleTypeDef *sac_adc_handle;
};

int stm32l432_adc_dev_init(struct os_dev *, void *);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_STM32L432_H__ */
