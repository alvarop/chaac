#include <assert.h>
#include <string.h>
#include <console/console.h>
#include <stdio.h>
#include <hal/hal_i2c.h>

#include <sysinit/sysinit.h>
#include <os/os.h>
#include <bsp/bsp.h>
#include <hal/hal_gpio.h>
#include <bme280/bme280.h>
#include <simple_adc/simple_adc.h>
#include <windrain/windrain.h>
#include <packet/packet.h>
#include <hal/hal_nvreg.h>
#include <host/ble_hs.h>

#include "chaac_packet.h"

struct os_callout sample_callout;
static uint8_t sample_num;

static volatile uint16_t timestamp;

#define BEACON_MAGIC 0xcaac

static weather_data_packet_small_t packet;

static void ble_app_advertise(void *packet, int32_t duration);

void weather_init() {
    hal_gpio_init_out(LED1_PIN, 1);

    hal_gpio_init_out(WX_DIR_EN_PIN, WX_DIR_EN_OFF);

    int32_t rval;

    rval = bme280_init();
    if(rval) {
        console_printf("Error initializing BME280 (%ld)\n", rval);
    }

    windrain_init();

    sample_num = 0;
}

const char * wind_dir_str[16] = {
    "N", "NNE", "NE", "ENE",
    "E", "ESE", "SE", "SSE",
    "S", "SSW", "SW", "WSW",
    "W", "WNW", "NW", "NNW"};

void weather_sample_fn(struct os_event *ev) {
    int32_t result = 0;
    int32_t rval;

    // schedule next event asap
    os_callout_reset(&sample_callout, OS_TICKS_PER_SEC * MYNEWT_VAL(CHAAC_SAMPLE_RATE_S));

    hal_gpio_write(LED1_PIN, 1);
    simple_adc_init();

    packet.sample = sample_num;

    hal_gpio_init_out(WX_DIR_EN_PIN, WX_DIR_EN_ON);
    // Measured rise time in WX_DIR pin ~1ms (with 0.1uF cap)
    // 5 ms delay is plenty to settle before measuring
    os_time_delay(os_time_ms_to_ticks32(5));

    rval = simple_adc_read_ch(BATT_ADC_CH, &result);
    if(rval) {
        console_printf("simple_adc_read_ch error %ld\n", rval);
    } else {
        packet.battery = result * 2;
        console_printf("B: %ld.%ld\n",
            (int32_t)(packet.battery/1000),
            (int32_t)((packet.battery-(int32_t)(packet.battery/1000)*1000)));
    }

    rval = simple_adc_read_ch(VSOLAR_ADC_CH, &result);
    if(rval) {
        console_printf("simple_adc_read_ch error %ld\n", rval);
    } else {
        packet.solar_panel = (float)result * 2;
        console_printf("S: %ld.%ld\n",
            (int32_t)(packet.solar_panel/1000),
            (int32_t)((packet.solar_panel-(int32_t)(packet.solar_panel/1000)*1000)));
    }

    float temperature, humidity, pressure;
    rval = bme280_read(&temperature, &pressure, &humidity);
    // pressure /= 100.0; // Convert to hPa
    if (rval) {
        console_printf("Error reading from BMP280 (%ld)\n", rval);
        // Set unrealistic values during error
        packet.pressure = 0;
        packet.temperature = -273;
        packet.humidity = 0;
    } else {
        // Convert to hPa difference from 1000.00 hPa (to use int16_t)
        packet.pressure = (int16_t)(((pressure - 100000.0)));
        packet.temperature = (int16_t)(temperature * 100.0);
        packet.humidity = (uint16_t)(humidity * 100.0);
        console_printf("P: %ld.%02ld T: %ld.%02ld H:%ld.%02ld\n",
            (int32_t)(pressure/100),
            (int32_t)((pressure/100-(int32_t)(pressure/100))*100),
            (int32_t)(temperature),
            (uint32_t)((temperature-(uint32_t)(temperature))*100),
            (int32_t)(packet.humidity/100),
            (int32_t)((packet.humidity-(int32_t)(packet.humidity/100)*100)));
    }

    // Store rain by multiples of 0.2794mm
    packet.rain = windrain_get_rain()/2794;

    // Store wind speed in kph * 100
    packet.wind_speed = windrain_get_speed()/10;

    packet.wind_dir = windrain_get_dir();
    hal_gpio_init_out(WX_DIR_EN_PIN, WX_DIR_EN_OFF);

    console_printf("ws: %ld.%ld kph %s\n",
            (int32_t)(packet.wind_speed/100),
            (int32_t)((packet.wind_speed-(int32_t)(packet.wind_speed/100)*100)),
            wind_dir_str[packet.wind_dir]);
    console_printf("rain: %ld.%ld mm\n",
            (int32_t)(packet.rain*2794/10000),
            (int32_t)((packet.rain*2794-(int32_t)((packet.rain*2794)/10000))*10000));

    sample_num++;

    // Turn off ADC for some small power savings
    simple_adc_uninit();
    hal_gpio_write(LED1_PIN, 0);

    ble_app_advertise(&packet, 1000);

    windrain_clear_rain();
}


static void ble_app_set_addr(void) {
    // Set non-random address so we can identify the beacon after resets
    // Still setting type to BLE_ADDR_RANDOM because an error occurs otherwise
    ble_addr_t addr;
    int rc;
    uint32_t uid = DEVICE_UID;
    addr.type = BLE_ADDR_RANDOM;
    addr.val[0] = ((uint8_t *)&uid)[0];
    addr.val[1] = ((uint8_t *)&uid)[1];
    addr.val[2] = ((uint8_t *)&uid)[2];
    addr.val[3] = ((uint8_t *)&uid)[3];
    addr.val[4] = 0;
    addr.val[5] = 0;

    rc = ble_hs_id_set_rnd(addr.val);
    assert(rc == 0);
}

static void ble_app_advertise(void *packet, int32_t duration) {
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    int rc;

    fields = (struct ble_hs_adv_fields){ 0 };
    fields.mfg_data_len = 1;
    fields.mfg_data = (uint8_t[]){ 0xca};

    rc = ble_eddystone_set_adv_data_uid(&fields, packet, 0);
    assert(rc == 0);

    adv_params = (struct ble_gap_adv_params){ 0 };
    rc = ble_gap_adv_start(
        BLE_OWN_ADDR_RANDOM,
        NULL,
        duration,
        &adv_params,
        NULL,
        NULL);
    assert(rc == 0);
}

static void ble_app_on_sync(void) {
    // Set address
    ble_app_set_addr();
}


int main(int argc, char **argv) {
    int rc;

    sysinit();

    console_printf("Chaac v%d.%d\n", (CHAAC_HW_VERS >> 8), CHAAC_HW_VERS & 0xFF);
    console_printf("UID: %08lX\n", DEVICE_UID);
    console_printf("Sample period: %ds\n", MYNEWT_VAL(CHAAC_SAMPLE_RATE_S));

    weather_init();

    os_callout_init(&sample_callout, os_eventq_dflt_get(), weather_sample_fn, NULL);
    os_callout_reset(&sample_callout, OS_TICKS_PER_SEC * MYNEWT_VAL(CHAAC_SAMPLE_RATE_S));

    ble_hs_cfg.sync_cb = ble_app_on_sync;

    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    assert(0);
    return rc;
}
