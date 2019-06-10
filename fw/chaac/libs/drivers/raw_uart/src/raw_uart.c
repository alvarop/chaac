
#include <raw_uart/raw_uart.h>
#include <uart/uart.h>
#include <bsp/bsp.h>
#include <hal/hal_gpio.h>
#include <fifo/fifo.h>
#include <os/os.h>

static struct uart_dev *uart_dev;

#define RAW_UART_RX_FIFO_SIZE (128)

static fifo_t rx_fifo;
static uint8_t rx_fifo_buff[RAW_UART_RX_FIFO_SIZE];

static struct os_event raw_uart_rx_ev;

static int raw_uart_tx_char(void *arg)
{
    // -1 means done transmitting
    // otherwise return char to tx...
    return -1;
}

static int raw_uart_rx_char(void *arg, uint8_t byte)
{
    fifo_push(&rx_fifo, byte);
    // signal packet processor that new data is available
    if(raw_uart_rx_ev.ev_cb != NULL) {
        os_eventq_put(os_eventq_dflt_get(), &raw_uart_rx_ev);
    }
    return 0;
}

int32_t raw_uart_init(void (*rx_ev_fn)(struct os_event *ev)) {
    int32_t rval = 0;

    struct uart_conf uc = {
        .uc_speed = MYNEWT_VAL(RAW_UART_BAUD),
        .uc_databits = 8,
        .uc_stopbits = 1,
        .uc_parity = UART_PARITY_NONE,
        .uc_flow_ctl = UART_FLOW_CTL_NONE,
        .uc_tx_char = raw_uart_tx_char,
        .uc_rx_char = raw_uart_rx_char,
    };

    fifo_init(&rx_fifo, RAW_UART_RX_FIFO_SIZE, rx_fifo_buff);

    if (rx_ev_fn != NULL) {
        raw_uart_rx_ev.ev_cb = rx_ev_fn;
        raw_uart_rx_ev.ev_arg = &rx_fifo;
    }

    if (!uart_dev) {
        uart_dev = (struct uart_dev *)os_dev_open(MYNEWT_VAL(RAW_UART_DEV),
          OS_TIMEOUT_NEVER, &uc);
        if (!uart_dev) {
            return -1;
        }
    }

    return rval;
}
int32_t raw_uart_tx(uint32_t len, void * buff) {
    int32_t rval = 0;
    uint8_t *byte = (uint8_t *)buff;

    assert(len > 0);

    while(len--){
        uart_blocking_tx(uart_dev, *byte++);
    }
    return rval;
}
