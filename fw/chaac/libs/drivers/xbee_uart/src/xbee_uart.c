
#include <xbee_uart/xbee_uart.h>
#include <uart/uart.h>
#include <bsp/bsp.h>
#include <fifo/fifo.h>
#include <os/os.h>

static struct uart_dev *uart_dev;

#define XBEE_RX_FIFO_SIZE (128)

static fifo_t rx_fifo;
static uint8_t rx_fifo_buff[XBEE_RX_FIFO_SIZE];

static struct os_event xbee_uart_rx_ev;

static int xbee_uart_tx_char(void *arg)
{
    // -1 means done transmitting
    // otherwise return char to tx...
    return -1;
}

static int xbee_uart_rx_char(void *arg, uint8_t byte)
{
    fifo_push(&rx_fifo, byte);
    // signal packet processor that new data is available
    if(xbee_uart_rx_ev.ev_cb != NULL) {
        os_eventq_put(os_eventq_dflt_get(), &xbee_uart_rx_ev);
    }
    return 0;
}

int32_t xbee_uart_init(void (*rx_ev_fn)(struct os_event *ev)) {
    int32_t rval = 0;

    struct uart_conf uc = {
        .uc_speed = MYNEWT_VAL(XBEE_UART_BAUD),
        .uc_databits = 8,
        .uc_stopbits = 1,
        .uc_parity = UART_PARITY_NONE,
        .uc_flow_ctl = UART_FLOW_CTL_NONE,
        .uc_tx_char = xbee_uart_tx_char,
        .uc_rx_char = xbee_uart_rx_char,
    };

    fifo_init(&rx_fifo, XBEE_RX_FIFO_SIZE, rx_fifo_buff);

    if (rx_ev_fn != NULL) {
        xbee_uart_rx_ev.ev_cb = rx_ev_fn;
        xbee_uart_rx_ev.ev_arg = &rx_fifo;
    }

    if (!uart_dev) {
        uart_dev = (struct uart_dev *)os_dev_open(MYNEWT_VAL(XBEE_UART_DEV),
          OS_TIMEOUT_NEVER, &uc);
        if (!uart_dev) {
            return -1;
        }
    }


    return rval;
}
int32_t xbee_uart_tx(uint32_t len, void * buff) {
    int32_t rval = 0;
    uint8_t *byte = (uint8_t *)buff;

    assert(len > 0);

    while(len--){
        uart_blocking_tx(uart_dev, *byte++);
    }
    return rval;
}
