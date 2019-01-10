
#include <xbee_uart/xbee_uart.h>
#include <uart/uart.h>
#include <bsp/bsp.h>

static struct uart_dev *uart_dev;

static int xbee_uart_tx_char(void *arg)
{
    //  TODO - implement this
    return ' ';
}

static int xbee_uart_rx_char(void *arg, uint8_t byte)
{
    // TODO - implement this
    return 0;
}

int32_t xbee_uart_init() {
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
