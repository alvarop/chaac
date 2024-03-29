#ifndef __PACKET_H__
#define __PACKET_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint16_t start;
    uint16_t len;
} __attribute__((packed)) packet_header_t;

typedef struct {
    uint16_t crc;
} __attribute__((packed)) packet_footer_t;

#define MAX_PACKET_DATA_LEN 272
#define MAX_PACKET_SIZE (sizeof(packet_header_t) + sizeof(packet_footer_t) + MAX_PACKET_DATA_LEN)
#define PACKET_START 0xAA55

void packetInitCb(void (*cb_fn)(uint16_t len, void *data));
void packetProcessByte(uint8_t byte);
int32_t packetTx(uint16_t len, void *data, void (*txFn)(uint16_t len, void *data));
bool packetIsValid(uint8_t *buffer, size_t len);

#endif
