#include <hal/hal_bsp.h>
#include <assert.h>
#include "os/mynewt.h"
#include <mcu/cmsis_nvic.h>
#include "stm32l4xx_hal_dma.h"
#include "stm32l4xx_hal_adc.h"
#include "stm32l4xx_hal_rcc.h"
#include "stm32l4xx_hal_cortex.h"
#include "stm32l4xx_hal.h"
#include "mcu/stm32l4xx_mynewt_hal.h"
#include <adc_stm32l432/adc_stm32l432.h>

#if MYNEWT_VAL(ADC_1)
#include <adc/adc.h>
#endif

static void
stm32l432_adc_clk_enable(ADC_HandleTypeDef *hadc)
{
    uintptr_t adc_addr = (uintptr_t)hadc->Instance;

    switch (adc_addr) {
#if defined(ADC1)
        case (uintptr_t)ADC1:
            __HAL_RCC_ADC_CLK_ENABLE();
            break;
#endif
        default:
            assert(0);
    }
}

static void
stm32l432_adc_clk_disable(ADC_HandleTypeDef *hadc)
{
    uintptr_t adc_addr = (uintptr_t)hadc->Instance;

    switch (adc_addr) {
#if defined(ADC1)
        case (uintptr_t)ADC1:
            __HAL_RCC_ADC_CLK_DISABLE();
            break;
#endif
        default:
            assert(0);
    }
}

static int
stm32l432_resolve_adc_gpio(ADC_HandleTypeDef *adc, uint8_t cnum,
        GPIO_InitTypeDef *gpio)
{
    uintptr_t adc_addr = (uintptr_t)adc->Instance;
    uint32_t pin;
    int rc;

    rc = OS_OK;
    switch (adc_addr) {
#if defined(ADC1)
        case (uintptr_t)ADC1:
            switch(cnum) {
                case 5:
                    pin = ADC1_IN5_PIN;
                    goto done;
                case 6:
                    pin = ADC1_IN6_PIN;
                    goto done;
                case 7:
                    pin = ADC1_IN7_PIN;
                    goto done;
                case 8:
                    pin = ADC1_IN8_PIN;
                    goto done;
                case 9:
                    pin = ADC1_IN9_PIN;
                    goto done;
                case 10:
                    pin = ADC1_IN10_PIN;
                    goto done;
                case 11:
                    pin = ADC1_IN11_PIN;
                    goto done;
                case 12:
                    pin = ADC1_IN12_PIN;
                    goto done;
                case 15:
                    pin = ADC1_IN15_PIN;
                    goto done;
                case 16:
                    pin = ADC1_IN16_PIN;
                    goto done;
            }
#endif
        default:
            rc = OS_EINVAL;
            return rc;
    }
done:
    *gpio = (GPIO_InitTypeDef) {
        .Pin = pin,
        .Mode = GPIO_MODE_ANALOG,
        .Pull = GPIO_NOPULL,
        .Alternate = pin
    };
    return rc;
}

static void
stm32l432_adc_init(struct adc_dev *dev)
{
    struct stm32l432_adc_dev_cfg *adc_config;
    ADC_HandleTypeDef *hadc;

    assert(dev);

    adc_config = (struct stm32l432_adc_dev_cfg *)dev->ad_dev.od_init_arg;
    hadc = adc_config->sac_adc_handle;

    stm32l432_adc_clk_enable(hadc);

    if (HAL_ADC_Init(hadc) != HAL_OK) {
        assert(0);
    }
}

static void
stm32l432_adc_uninit(struct adc_dev *dev)
{
    GPIO_InitTypeDef gpio_td;
    ADC_HandleTypeDef *hadc;
    struct stm32l432_adc_dev_cfg *cfg;
    uint8_t cnum;

    assert(dev);
    cfg  = (struct stm32l432_adc_dev_cfg *)dev->ad_dev.od_init_arg;
    hadc = cfg->sac_adc_handle;
    cnum = dev->ad_chans->c_cnum;

    stm32l432_adc_clk_disable(hadc);

    if (stm32l432_resolve_adc_gpio(hadc, cnum, &gpio_td)) {
        goto err;
    }

    if (hal_gpio_deinit_stm(gpio_td.Pin, &gpio_td)) {
        goto err;
    }

err:
    return;
}

/**
 * Open the STM32L432 ADC device
 *
 * This function locks the device for access from other tasks.
 *
 * @param odev The OS device to open
 * @param wait The time in MS to wait.  If 0 specified, returns immediately
 *             if resource unavailable.  If OS_WAIT_FOREVER specified, blocks
 *             until resource is available.
 * @param arg  Argument provided by higher layer to open.
 *
 * @return 0 on success, non-zero on failure.
 */
static int
stm32l432_adc_open(struct os_dev *odev, uint32_t wait, void *arg)
{
    struct adc_dev *dev;
    int rc;

    assert(odev);
    rc = OS_OK;
    dev = (struct adc_dev *) odev;

    if (os_started()) {
        rc = os_mutex_pend(&dev->ad_lock, wait);
        if (rc != OS_OK) {
            goto err;
        }
    }

    if (odev->od_flags & OS_DEV_F_STATUS_OPEN) {
        os_mutex_release(&dev->ad_lock);
        rc = OS_EBUSY;
        goto err;
    }

    stm32l432_adc_init(dev);

    return (OS_OK);
err:
    return (rc);
}


/**
 * Close the STM32L432 ADC device.
 *
 * This function unlocks the device.
 *
 * @param odev The device to close.
 */
static int
stm32l432_adc_close(struct os_dev *odev)
{
    struct adc_dev *dev;

    dev = (struct adc_dev *) odev;

    stm32l432_adc_uninit(dev);

    if (os_started()) {
        os_mutex_release(&dev->ad_lock);
    }

    return (OS_OK);
}

/**
 * Configure an ADC channel on the STM32L432 ADC.
 *
 * @param dev The ADC device to configure
 * @param cnum The channel on the ADC device to configure
 * @param cfgdata An opaque pointer to channel config, expected to be
 *                a ADC_ChannelConfTypeDef
 *
 * @return 0 on success, non-zero on failure.
 */
static int
stm32l432_adc_configure_channel(struct adc_dev *dev, uint8_t cnum,
        void *cfgdata)
{
    int rc;
    ADC_HandleTypeDef *hadc;
    struct stm32l432_adc_dev_cfg *cfg;
    struct adc_chan_config *chan_cfg;
    GPIO_InitTypeDef gpio_td;

    cfg  = (struct stm32l432_adc_dev_cfg *)dev->ad_dev.od_init_arg;
    hadc = cfg->sac_adc_handle;

    rc = OS_EINVAL;

    if (dev == NULL && !IS_ADC_CHANNEL(hadc, cnum)) {
        goto err;
    }

    chan_cfg = &((struct adc_chan_config *)cfg->sac_chans)[cnum];

    cfgdata = (ADC_ChannelConfTypeDef *)cfgdata;

    if ((HAL_ADC_ConfigChannel(hadc, cfgdata)) != HAL_OK) {
        goto err;
    }

    dev->ad_chans[cnum].c_res = chan_cfg->c_res;
    dev->ad_chans[cnum].c_refmv = chan_cfg->c_refmv;
    dev->ad_chans[cnum].c_configured = 1;
    dev->ad_chans[cnum].c_cnum = cnum;

    if (stm32l432_resolve_adc_gpio(hadc, cnum, &gpio_td)) {
        goto err;
    }

    hal_gpio_init_stm(gpio_td.Pin, &gpio_td);

    return (OS_OK);
err:
    return (rc);
}


static int
stm32l432_adc_sample(struct adc_dev *dev)
{
    int rc;
    ADC_HandleTypeDef *hadc;
    struct stm32l432_adc_dev_cfg *cfg;

    assert(dev);
    cfg  = (struct stm32l432_adc_dev_cfg *)dev->ad_dev.od_init_arg;
    hadc = cfg->sac_adc_handle;

    rc = OS_EINVAL;

    if (HAL_ADC_Start(hadc) != HAL_OK) {
        goto err;
    }

    rc = OS_OK;

err:
    return rc;
}

/**
 * Blocking read of an ADC channel, returns result as an integer.
 *
 * @param1 ADC device structure
 * @param2 channel number
 * @param3 ADC result ptr
 */
static int
stm32l432_adc_read_channel(struct adc_dev *dev, uint8_t cnum, int *result)
{
    ADC_HandleTypeDef *hadc;
    struct stm32l432_adc_dev_cfg *cfg;

    assert(dev != NULL && result != NULL);
    cfg  = (struct stm32l432_adc_dev_cfg *)dev->ad_dev.od_init_arg;
    hadc = cfg->sac_adc_handle;

    if(HAL_ADC_PollForConversion(hadc, 100) != HAL_OK) {
        printf("Error waiting :(\n");
    }

    *result = HAL_ADC_GetValue(hadc);

    return (OS_OK);
}

/**
 * ADC device driver functions
 */
static const struct adc_driver_funcs stm32l432_adc_funcs = {
        .af_configure_channel = stm32l432_adc_configure_channel,
        .af_sample = stm32l432_adc_sample,
        .af_read_channel = stm32l432_adc_read_channel,
        .af_set_buffer = NULL,
        .af_release_buffer = NULL,
        .af_read_buffer = NULL,
        .af_size_buffer = NULL,
};

/**
 * Callback to initialize an adc_dev structure from the os device
 * initialization callback.  This sets up a stm32l432_adc_device(), so
 * that subsequent lookups to this device allow us to manipulate it.
 *
 * @param1 os device ptr
 * @param2 STM32L432 ADC device cfg ptr
 * @return OS_OK on success
 */
int
stm32l432_adc_dev_init(struct os_dev *odev, void *arg)
{
    struct stm32l432_adc_dev_cfg *sac;
    struct adc_dev *dev;

    sac = (struct stm32l432_adc_dev_cfg *) arg;

    assert(sac != NULL);

    dev = (struct adc_dev *)odev;

    os_mutex_init(&dev->ad_lock);

    dev->ad_chans = (void *) sac->sac_chans;
    dev->ad_chan_count = sac->sac_chan_count;

    OS_DEV_SETHANDLERS(odev, stm32l432_adc_open, stm32l432_adc_close);

    dev->ad_funcs = &stm32l432_adc_funcs;

    return (OS_OK);
}

