#ifndef __PACKET_H__
#define __PACKET_H__

#include <stdint.h>

typedef struct {
    uint16_t start;
    uint16_t len;
} __attribute__((packed)) packet_header_t;

typedef struct {
    uint16_t crc;
} __attribute__((packed)) packet_footer_t;

#define MAX_PACKET_DATA_LEN 90
#define MAX_PACKET_SIZE (sizeof(packet_header_t) + sizeof(packet_footer_t) + MAX_PACKET_DATA_LEN)
#define PACKET_START 0xAA55

int32_t packet_init_cb(void (*cb_fn)(int16_t len, void *data));
int32_t packet_init_tx_fn(void (*tx_fn)(int16_t len, void *data));
int32_t packet_process_byte(uint8_t byte);
int32_t packet_tx(uint16_t len, void *data);


#endif
