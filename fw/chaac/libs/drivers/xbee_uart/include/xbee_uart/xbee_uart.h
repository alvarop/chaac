#ifndef __XBEE_UART_H__
#define __XBEE_UART_H__

#include <stdint.h>

int32_t xbee_uart_init();
int32_t xbee_uart_tx(uint32_t len, void * buff);

#endif