#include <assert.h>
#include <string.h>
#include <console/console.h>
#include <stdio.h>
#include <hal/hal_i2c.h>

#include "sysinit/sysinit.h"
#include <os/os.h>
#include "bsp/bsp.h"
#include <hal/hal_gpio.h>
#include <am2315/am2315.h>
#include <bmp280/bmp280.h>
#include <simple_adc/simple_adc.h>
#include <windrain/windrain.h>
#include <xbee_uart/xbee_uart.h>
#include <packet/packet.h>
#include <packet/crc.h>

/* Define task stack and task object */
#define WEATHER_TASK_PRI         (10)
#define WEATHER_STACK_SIZE       (256)
struct os_task weather_task;
os_stack_t weather_task_stack[WEATHER_STACK_SIZE];

static uint8_t packet_tx_buff[MAX_PACKET_SIZE];

void xbee_enable(uint32_t timeout) {

    hal_gpio_write(XBEE_nSBY_PIN, 0);

    while((hal_gpio_read(XBEE_ON_PIN) == 0) && (timeout-- > 0)){
        // TODO - make this smarter
        // interrupt, actual timeout, sleep, etc
    }

    return;
}

void xbee_disable() {
    hal_gpio_write(XBEE_nSBY_PIN, 1);
    return;
}

int32_t packet_tx(uint16_t len, void *data) {
    int32_t rval = 0;
    do {
        if(len > MAX_DATA_LEN) {
            rval = -1;
            break;
        }
        crc_t crc;

        packet_header_t *header = (packet_header_t *)packet_tx_buff;
        packet_footer_t *footer = (packet_footer_t *)((uint8_t *)&header[1] + len);

        header->start = PACKET_START;
        header->len = len;

        memcpy((void*)&header[1], data, len);

        crc = crc_init();
        crc = crc_update(crc, header, header->len + sizeof(packet_header_t));
        crc = crc_finalize(crc);

        footer->crc = crc;

        // Seems to take ~500-600 'units' to wake up. 1000 to be safe
        xbee_enable(1000);
        xbee_uart_tx(
            header->len + sizeof(packet_footer_t) + sizeof(packet_header_t),
            packet_tx_buff);

        // TODO - figure out better way to know when to go back to standby
        // 2 os ticks is too short and it takes 3 packets to buffer before tx
        // 6 seems to be working ok...
        os_time_delay(6);
        xbee_disable();
    } while(0);


    return rval;
}

static uint32_t *uid = (uint32_t *)(0x1FFF7590);

void weather_task_func(void *arg) {
    printf("Weather Breakout!\n");

    hal_gpio_init_out(FAN_EN_PIN, 0);
    hal_gpio_init_in(XBEE_ON_PIN, HAL_GPIO_PULL_DOWN);
    hal_gpio_init_out(XBEE_nSBY_PIN, 1); // XBEE nSBY
    hal_gpio_init_out(LED1_PIN, 1);
    int32_t rval;

    am2315_init();
    rval = bmp280_init();
    if(rval) {
        printf("Error initializing BMP280 (%ld)\n", rval);
    }

    simple_adc_init();

    windrain_init();

    xbee_uart_init();

    while (1) {
        int32_t result = 0;
        os_time_delay(OS_TICKS_PER_SEC * 60);

        hal_gpio_write(LED1_PIN, 1);

        weather_data_packet_t packet;

        packet.uid = uid[0] ^ uid[1] ^ uid[2];
        packet.type = PACKET_TYPE_DATA;

        rval = simple_adc_read_ch(10, &result);
        if(rval) {
            printf("simple_adc_read_ch error %ld\n", rval);
        } else {
            packet.battery = (float)result * 2.0 / 1000.0;
            printf("Batt: %ld.%ld\n",
                (int32_t)(packet.battery),
                (int32_t)((packet.battery-(int32_t)(packet.battery))*1000));
        }

        rval = simple_adc_read_ch(11, &result);
        if(rval) {
            printf("simple_adc_read_ch error %ld\n", rval);
        } else {
            packet.light = (float)result / 1000.0;
            printf("Light: %ld.%ld\n",
                (int32_t)(packet.light),
                (int32_t)((packet.light-(int32_t)(packet.light))*1000));
        }

        rval = am2315_read(&packet.temperature, &packet.humidity);
        if (rval) {
            printf("Error reading from AM2315 (%ld)\n", rval);
        } else {
            printf("%ld.%02ld %ld.%02ld\n",
                (int32_t)(packet.humidity),
                (int32_t)((packet.humidity-(int32_t)(packet.humidity))*10),
                (int32_t)(packet.temperature),
                (uint32_t)((packet.temperature-(uint32_t)(packet.temperature))*10));
        }

        rval = bmp280_read(&packet.temperature_in, &packet.pressure);
        packet.pressure /= 100.0;
        if (rval) {
            printf("Error reading from BMP280 (%ld)\n", rval);
        } else {
            printf("%ld.%02ld %ld.%02ld\n",
                (int32_t)(packet.pressure),
                (int32_t)((packet.pressure-(int32_t)(packet.pressure))*100),
                (int32_t)(packet.temperature_in),
                (uint32_t)((packet.temperature_in-(uint32_t)(packet.temperature_in))*100));
        }

        packet.rain = (float)windrain_get_rain()/10000.0; // TODO check if 1000
        packet.wind_speed = (float)windrain_get_speed()/1000.0;
        packet.wind_dir = (float)windrain_get_dir()/10.0;

        printf( "ws: %ld.%ld kph @ %ld.%ld\n",
                (int32_t)(packet.wind_speed),
                (int32_t)((packet.wind_speed-(int32_t)(packet.wind_speed))*10),
                (int32_t)(packet.wind_dir),
                (int32_t)((packet.wind_dir-(int32_t)(packet.wind_dir))*10));
        printf("rain: %ld.%ld mm\n",
                (int32_t)(packet.rain),
                (int32_t)((packet.rain-(int32_t)(packet.rain))*10000));

        packet_tx(sizeof(weather_data_packet_t), (void*)&packet);

        hal_gpio_write(LED1_PIN, 0);
    }
}

int main(int argc, char **argv) {
    int rc;

    sysinit();

    os_task_init(
        &weather_task,
        "weather_task",
        weather_task_func,
        NULL,
        WEATHER_TASK_PRI,
        OS_WAIT_FOREVER,
        weather_task_stack,
        WEATHER_STACK_SIZE);

    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    assert(0);
    return rc;
}
