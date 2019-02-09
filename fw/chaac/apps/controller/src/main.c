#include <assert.h>
#include <string.h>
#include <console/console.h>
#include <stdio.h>
#include <hal/hal_i2c.h>

#include <sysinit/sysinit.h>
#include <os/os.h>
#include <bsp/bsp.h>
#include <hal/hal_gpio.h>
#include <am2315/am2315.h>
#include <bmp280/bmp280.h>
#include <simple_adc/simple_adc.h>
#include <windrain/windrain.h>
#include <xbee_uart/xbee_uart.h>
#include <packet/packet.h>
#include <packet/crc.h>
#include <fifo/fifo.h>
#include <hal/hal_nvreg.h>

#include "chaac_packet.h"

// XOR uid 32-bit words to have a single 32-bit uid
#define DEVICE_UID (   ((uint32_t *)(0x1FFF7590))[0] ^ \
                        ((uint32_t *)(0x1FFF7590))[1] ^ \
                        ((uint32_t *)(0x1FFF7590))[2])

struct os_callout sample_callout;
static uint32_t sample_num;

void packet_tx_fn(int16_t len, void* data) {
    // Seems to take ~500-600 'units' to wake up. 1000 to be safe
    xbee_enable(1000);

    xbee_uart_tx(len, data);

    // Wait ~124ms for response packets
    // This is enough time to get a response if any
    os_time_delay(OS_TICKS_PER_SEC/8);
    xbee_disable();
}

void xbee_rx_handler(struct os_event *ev) {
    fifo_t *fifo = ev->ev_arg;

    while(fifo_size(fifo)) {
        packet_process_byte(fifo_pop(fifo));
    }
}

void chaac_packet_handler(int16_t len, void* data) {
    chaac_header_t *header = (chaac_header_t*)data;

    do {
        if (header->uid != DEVICE_UID) {
            break;
        }

        switch(header->type) {
            case PACKET_TYPE_BOOT: {
                 // Reset to bootloader
                hal_nvreg_write(0, 0xB7);
                NVIC_SystemReset();
                break;
            }

            case PACKET_TYPE_CLEAR_RAIN: {
                windrain_clear_rain();
                break;
            }

            default: {
                break;
            }
        }
    } while(0);

}

void weather_init() {
    hal_gpio_init_out(LED1_PIN, 1);

    hal_gpio_init_out(WX_DIR_EN_PIN, WX_DIR_EN_OFF);

    int32_t rval;

    am2315_init();
    rval = bmp280_init();
    if(rval) {
        console_printf("Error initializing BMP280 (%ld)\n", rval);
    }


    windrain_init();

    xbee_uart_init(&xbee_rx_handler);
    packet_init_cb(chaac_packet_handler);
    packet_init_tx_fn(packet_tx_fn);

    sample_num = 0;
}

void weather_sample_fn(struct os_event *ev) {
    int32_t result = 0;
    int32_t rval;

    // schedule next event asap
    os_callout_reset(&sample_callout, OS_TICKS_PER_SEC * MYNEWT_VAL(CHAAC_SAMPLE_RATE_S));

    hal_gpio_write(LED1_PIN, 1);
    simple_adc_init();

    weather_data_packet_t packet;

    packet.header.uid = DEVICE_UID;
    packet.header.type = PACKET_TYPE_DATA;

    packet.sample = sample_num;

    hal_gpio_init_out(WX_DIR_EN_PIN, WX_DIR_EN_ON);
    // Measured rise time in WX_DIR pin ~1ms (with 0.1uF cap)
    // 5 ms delay is plenty to settle before measuring
    os_time_delay(os_time_ms_to_ticks32(5));

    rval = simple_adc_read_ch(BATT_ADC_CH, &result);
    if(rval) {
        console_printf("simple_adc_read_ch error %ld\n", rval);
    } else {
        packet.battery = (float)result * 2.0 / 1000.0;
        console_printf("B: %ld.%ld\n",
            (int32_t)(packet.battery),
            (int32_t)((packet.battery-(int32_t)(packet.battery))*1000));
    }

    rval = simple_adc_read_ch(LIGHT_ADC_CH, &result);
    if(rval) {
        console_printf("simple_adc_read_ch error %ld\n", rval);
    } else {
        packet.light = (float)result / 3300.0; // Normalize from 0.0-1.0
        console_printf("L: %ld.%ld\n",
            (int32_t)(packet.light),
            (int32_t)((packet.light-(int32_t)(packet.light))*1000));
    }

#if CHAAC_HW_VERS > 0x101
    rval = simple_adc_read_ch(VSOLAR_ADC_CH, &result);
    if(rval) {
        console_printf("simple_adc_read_ch error %ld\n", rval);
    } else {
        packet.solar_panel = (float)result * 2.0 / 1000.0;
        console_printf("S: %ld.%ld\n",
            (int32_t)(packet.solar_panel),
            (int32_t)((packet.solar_panel-(int32_t)(packet.solar_panel))*1000));
    }
#else
    packet.solar_panel = 0;
#endif

    rval = am2315_read(&packet.temperature, &packet.humidity);
    if (rval) {
        console_printf("Error reading from AM2315 (%ld)\n", rval);
        // Set unrealistic values during error
        // Absolute zero temp and zero humidity
        packet.temperature = -273.15;
        packet.humidity = 0;
    } else {
        console_printf("H:%ld.%02ld T:%ld.%02ld\n",
            (int32_t)(packet.humidity),
            (int32_t)((packet.humidity-(int32_t)(packet.humidity))*10),
            (int32_t)(packet.temperature),
            (uint32_t)((packet.temperature-(uint32_t)(packet.temperature))*10));
    }

    rval = bmp280_read(&packet.temperature_in, &packet.pressure);
    packet.pressure /= 100.0;
    if (rval) {
        console_printf("Error reading from BMP280 (%ld)\n", rval);
        // Set unrealistic values during error
        packet.pressure = 0;
        packet.temperature_in = -273.15;
    } else {
        console_printf("P: %ld.%02ld T: %ld.%02ld\n",
            (int32_t)(packet.pressure),
            (int32_t)((packet.pressure-(int32_t)(packet.pressure))*100),
            (int32_t)(packet.temperature_in),
            (uint32_t)((packet.temperature_in-(uint32_t)(packet.temperature_in))*100));
    }

    packet.rain = (float)windrain_get_rain()/10000.0; // TODO check if 1000
    packet.wind_speed = (float)windrain_get_speed()/1000.0;


    packet.wind_dir = (float)windrain_get_dir()/10.0;
    hal_gpio_init_out(WX_DIR_EN_PIN, WX_DIR_EN_OFF);

    console_printf("ws: %ld.%ld kph @ %ld.%ld\n",
            (int32_t)(packet.wind_speed),
            (int32_t)((packet.wind_speed-(int32_t)(packet.wind_speed))*10),
            (int32_t)(packet.wind_dir),
            (int32_t)((packet.wind_dir-(int32_t)(packet.wind_dir))*10));
    console_printf("rain: %ld.%ld mm\n",
            (int32_t)(packet.rain),
            (int32_t)((packet.rain-(int32_t)(packet.rain))*10000));

    packet_tx(sizeof(weather_data_packet_t), (void*)&packet);

    sample_num++;

    // Turn off ADC for some small power savings
    simple_adc_uninit();
    hal_gpio_write(LED1_PIN, 0);

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

    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    assert(0);
    return rc;
}
