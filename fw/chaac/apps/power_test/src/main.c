#include <assert.h>
#include <string.h>
#include <console/console.h>
#include <stdio.h>
// #include <hal/hal_i2c.h>

#include <sysinit/sysinit.h>
#include <os/os.h>
#include <bsp/bsp.h>
#include <hal/hal_gpio.h>
// #include <am2315/am2315.h>
// #include <bmp280/bmp280.h>
// #include <simple_adc/simple_adc.h>
// #include <windrain/windrain.h>
// #include <xbee_uart/xbee_uart.h>
// #include <packet/packet.h>
// #include <packet/crc.h>
// #include <fifo/fifo.h>
#include <hal/hal_nvreg.h>

#include "chaac_packet.h"

// XOR uid 32-bit words to have a single 32-bit uid
#define DEVICE_UID (   ((uint32_t *)(0x1FFF7590))[0] ^ \
                        ((uint32_t *)(0x1FFF7590))[1] ^ \
                        ((uint32_t *)(0x1FFF7590))[2])

struct os_callout sample_callout;
extern uint32_t SystemCoreClock;

void weather_init() {
    hal_gpio_init_out(FAN_EN_PIN, 0);
    hal_gpio_init_in(XBEE_ON_PIN, HAL_GPIO_PULL_DOWN);

    hal_gpio_init_out(LED1_PIN,0);

    hal_gpio_init_out(LIGHT_PIN, 0);
    hal_gpio_init_out(XBEE_nSBY_PIN, 0); // XBEE nSBY

    // xbee_uart_init(&xbee_rx_handler);
}

void weather_sample_fn(struct os_event *ev) {

    // schedule next event asap
    os_callout_reset(&sample_callout, OS_TICKS_PER_SEC * 2);

    hal_gpio_write(LED1_PIN, 1);
    hal_gpio_write(LIGHT_PIN, 1);
    os_time_delay(100);
    hal_gpio_write(LED1_PIN, 0);
    hal_gpio_write(LIGHT_PIN, 0);
}

int main(int argc, char **argv) {
    int rc;

    sysinit();

    volatile uint32_t speed = SystemCoreClock;

    console_printf("Chaac v%d.%d\n", (CHAAC_HW_VERS >> 8), CHAAC_HW_VERS & 0xFF);
    console_printf("UID: %08lX\n", DEVICE_UID);
    console_printf("Sample period: %ds\n", MYNEWT_VAL(CHAAC_SAMPLE_RATE_S));
    console_printf("SPEED: %ld\n", speed);

    weather_init();

    os_callout_init(&sample_callout, os_eventq_dflt_get(), weather_sample_fn, NULL);
    os_callout_reset(&sample_callout, OS_TICKS_PER_SEC * MYNEWT_VAL(CHAAC_SAMPLE_RATE_S));

    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    assert(0);
    return rc;
}
