#ifndef __XBEE_UART_H__
#define __XBEE_UART_H__

#include <stdint.h>
#include <os/os.h>

void xbee_enable(uint32_t timeout);
void xbee_disable();
int32_t xbee_uart_init(void (*rx_ev_fn)(struct os_event *ev));
int32_t xbee_uart_tx(uint32_t len, void * buff);

#endif
