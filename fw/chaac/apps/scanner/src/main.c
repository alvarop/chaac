#include <assert.h>
#include <string.h>

#include "sysinit/sysinit.h"
#include "os/os.h"
#include "os/os_cputime.h"
#include "hal/hal_gpio.h"
#include "hal/hal_timer.h"
#include "bsp/bsp.h"
#include <host/ble_hs.h>
#include <packet/packet.h>
#include <raw_uart/raw_uart.h>

#define LED_TASK_PRI         (99)
#define LED_STACK_SIZE       (128)
struct os_task led_task;
os_stack_t led_task_stack[LED_STACK_SIZE];


static int ble_app_gap_event(struct ble_gap_event *event, void *arg);

#define BEACON_MAGIC 0xCAAC

typedef struct {
    uint16_t magic;
    uint32_t device_id;
    uint8_t fill[10];
} __attribute__((packed)) ble_beacon_t;


static void ble_app_scan(void) {
    uint8_t own_addr_type;
    struct ble_gap_disc_params disc_params;
    int rc;

    /* Figure out address to use while advertising (no privacy for now) */
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0) {
        MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
        return;
    }

    // Don't discard any packets!
    disc_params.filter_duplicates = 0;

    // Don't send follow-up requests
    disc_params.passive = 1;

    /* Use defaults for the rest of the parameters. */
    disc_params.itvl = 0;
    disc_params.window = 0;
    disc_params.filter_policy = 0;
    disc_params.limited = 0;

    rc = ble_gap_disc(own_addr_type, BLE_HS_FOREVER, &disc_params,
                      ble_app_gap_event, NULL);
    if (rc != 0) {
        MODLOG_DFLT(ERROR, "Error initiating GAP discovery procedure; rc=%d\n",
                    rc);
    }
}

typedef struct {
    uint8_t addr[6];
    uint8_t data[16];
    int8_t rssi;
} eddystone_packet_t;

eddystone_packet_t packet;

static int ble_app_gap_event(struct ble_gap_event *event, void *arg) {

    struct ble_hs_adv_fields fields;
    int rc;

    switch (event->type) {
    case BLE_GAP_EVENT_DISC:
        rc = ble_hs_adv_parse_fields(&fields, event->disc.data,
                                     event->disc.length_data);
        if (rc != 0) {
            return 0;
        }

        if (fields.uuids16 != NULL &&
            fields.uuids16->value == 0xFEAA &&
            fields.mfg_data_len == 1 &&
            fields.mfg_data[0] == 0xca) {
            memcpy(&packet.addr, event->disc.addr.val, sizeof(packet.addr));
            memcpy(&packet.data, &fields.svc_data_uuid16[4], sizeof(packet.data));
            packet.rssi = event->disc.rssi;

            packet_tx(sizeof(eddystone_packet_t), &packet);
        }

        return 0;

    default:
        return 0;
    }
}

static void ble_app_set_addr(void) {
    ble_addr_t addr;
    int rc;

    rc = ble_hs_id_gen_rnd(1, &addr);
    assert(rc == 0);

    rc = ble_hs_id_set_rnd(addr.val);
    assert(rc == 0);
}

static void ble_app_on_sync(void) {

    ble_app_set_addr();

    ble_app_scan();
}


void led_task_fn(void *arg) {

    hal_gpio_init_out(LED_BLINK_PIN, 0);
    hal_gpio_init_out(RX_LED_PIN, 0);

    while(1) {
        os_time_delay(OS_TICKS_PER_SEC*30);
        hal_gpio_write(LED_BLINK_PIN, 1);
        os_time_delay(OS_TICKS_PER_SEC/64);
        hal_gpio_write(LED_BLINK_PIN, 0);
    }
}

void packet_tx_fn(int16_t len, void* data) {
    hal_gpio_init_out(RX_LED_PIN, 1);
    raw_uart_tx(len, data);
    hal_gpio_init_out(RX_LED_PIN, 0);
}

int
main(int argc, char **argv)
{
    sysinit();

    os_task_init(
        &led_task,
        "led_task",
        led_task_fn,
        NULL,
        LED_TASK_PRI,
        OS_WAIT_FOREVER,
        led_task_stack,
        LED_STACK_SIZE);


    ble_hs_cfg.sync_cb = ble_app_on_sync;

    raw_uart_init(NULL);
    packet_init_tx_fn(packet_tx_fn);

    while(1) {
        os_eventq_run(os_eventq_dflt_get());
    }

    assert(0);

    return 0;
}
