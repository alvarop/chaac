#include <assert.h>
#include <string.h>

#include "sysinit/sysinit.h"
#include "os/os.h"
#include "console/console.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"

#include <host/ble_hs.h>

#define BLINK_TASK_PRI         (99)
#define BLINK_STACK_SIZE       (64)
struct os_task blink_task;
os_stack_t blink_task_stack[BLINK_STACK_SIZE];

static volatile uint16_t timestamp;

#define BEACON_MAGIC 0xcaac

typedef struct {
    uint8_t     index;
    uint8_t     wind_dir;
    uint8_t     rain;
    uint8_t     rsvd;
    uint16_t    wind_speed;
    int16_t     temperature;
    uint16_t    humidity;
    uint16_t    pressure;
    uint16_t    battery;
    uint16_t    solar_panel;
} __attribute__((packed)) weather_data_packet_small_t;

static weather_data_packet_small_t packet;

static void ble_app_advertise();

static void ble_app_set_addr(void) {
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

static void ble_app_advertise() {
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    int rc;

    packet.index = 123;
    packet.wind_dir = 1;
    packet.rain = 2;
    packet.rsvd = 3;
    packet.wind_speed = 4;
    packet.temperature = 5;
    packet.humidity = 6;
    packet.pressure = 7;
    packet.battery = 8;
    packet.solar_panel = 9;

    fields = (struct ble_hs_adv_fields){ 0 };
    fields.mfg_data_len = 1;
    fields.mfg_data = (uint8_t[]){ 0xca};

    rc = ble_eddystone_set_adv_data_uid(&fields, &packet, 0);
    assert(rc == 0);

    adv_params = (struct ble_gap_adv_params){ 0 };
    rc = ble_gap_adv_start(
        BLE_OWN_ADDR_RANDOM,
        NULL,
        BLE_HS_FOREVER,
        &adv_params,
        NULL,
        NULL);
    assert(rc == 0);
}

static void ble_app_on_sync(void) {
    /* Generate a non-resolvable private address. */
    ble_app_set_addr();

    ble_app_advertise();
}




void blink_task_fn(void *arg) {

    hal_gpio_init_out(LED_BLINK_PIN, 0);

    while(1) {
        os_time_delay(OS_TICKS_PER_SEC);
        hal_gpio_toggle(LED_BLINK_PIN);
    }

}

int
main(int argc, char **argv)
{
    sysinit();

    os_task_init(
        &blink_task,
        "blink_task",
        blink_task_fn,
        NULL,
        BLINK_TASK_PRI,
        OS_WAIT_FOREVER,
        blink_task_stack,
        BLINK_STACK_SIZE);

    ble_hs_cfg.sync_cb = ble_app_on_sync;

    while(1) {
        os_eventq_run(os_eventq_dflt_get());
    }

    assert(0);

    return 0;
}
