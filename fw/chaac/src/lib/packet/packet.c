#include <stdio.h>
#include <string.h>
#include "packet.h"
#include "crc.h"

static uint8_t packet_tx_buff[MAX_PACKET_SIZE];
static uint8_t rx_buff[MAX_PACKET_SIZE];
static uint32_t buff_index = 0;
static packet_header_t *header = (packet_header_t*)(rx_buff);

typedef enum {
    PACKET_STATE_CLEAR,
    PACKET_STATE_START,
    PACKET_STATE_LEN,
    PACKET_STATE_DATA,
    PACKET_STATE_CRC
} packet_state_t;

static packet_state_t state = PACKET_STATE_CLEAR;

static void (*packet_cb_fn)(uint16_t len, void *data) = NULL;

void packetInitCb(void (*cb_fn)(uint16_t len, void *data)) {
    packet_cb_fn = cb_fn;
}

void packetProcessByte(uint8_t byte) {
    switch(state) {
        case PACKET_STATE_CLEAR: {
            if (byte == (PACKET_START & 0xFF)) {
                rx_buff[buff_index++] = byte;
                state = PACKET_STATE_START;
            } else {
                buff_index = 0;
                state = PACKET_STATE_CLEAR;
            }
            break;
        }

        case PACKET_STATE_START: {
            if (byte == ((PACKET_START >> 8) & 0xFF)) {
                rx_buff[buff_index++] = byte;
                state = PACKET_STATE_LEN;
            } else {
                buff_index = 0;
                state = PACKET_STATE_CLEAR;
            }
            break;
        }

        case PACKET_STATE_LEN: {
            rx_buff[buff_index++] = byte;

            if (buff_index < sizeof(packet_header_t)) {
                // Need one more len byte
                break;
            }

            if (header->len > MAX_PACKET_DATA_LEN) {
                buff_index = 0;
                state = PACKET_STATE_CLEAR;
                break;
            }

            state = PACKET_STATE_DATA;
            break;
        }

        case PACKET_STATE_DATA: {
            rx_buff[buff_index++] = byte;

            if (buff_index < (sizeof(packet_header_t) + header->len)) {
                // Need more data bytes
                break;
            }

            state = PACKET_STATE_CRC;
            break;
        }

        case PACKET_STATE_CRC: {
            rx_buff[buff_index++] = byte;

            if (buff_index < (sizeof(packet_header_t) +
                                header->len + sizeof(packet_footer_t))) {
                // Need one more crc byte
                break;
            }

            packet_footer_t *footer = (packet_footer_t *)(
                                        (uint8_t *)&header[1] + header->len);

            crc_t crc = crc_init();
            crc = crc_update(crc, header, header->len + sizeof(packet_header_t));
            crc = crc_finalize(crc);

            if (crc == footer->crc) {
                if (packet_cb_fn != NULL) {
                    // Process packet
                    packet_cb_fn(header->len, (void *)&header[1]);
                }
            }

            buff_index = 0;
            state = PACKET_STATE_CLEAR;
            break;
        }
    }
}

bool packetIsValid(uint8_t *buffer, size_t len) {
    bool valid = false;
    do {
        packet_header_t *header = (packet_header_t *)buffer;

        // Not enough data for header
        if (len < (sizeof(packet_header_t) + sizeof(packet_footer_t))) {
            break;
        }

        // Invalid packet start
        if (header->start != PACKET_START) {
            break;
        }

        // Invalid packet len
        if (header->len > MAX_PACKET_DATA_LEN) {
            break;
        }

        // Still not enough data total
        if (len < (sizeof(packet_header_t) + sizeof(packet_footer_t) + header->len)) {
            break;
        }

        packet_footer_t *footer = (packet_footer_t *)(
                                        (uint8_t *)&header[1] + header->len);

        crc_t crc = crc_init();
        crc = crc_update(crc, header, header->len + sizeof(packet_header_t));
        crc = crc_finalize(crc);

        if(crc != footer->crc) {
            break;
        }

        valid = true;
    } while(0);

    return valid;
}

int32_t packetTx(uint16_t len, void *data, void (*txFn)(uint16_t len, void *data)) {
    int32_t rval = 0;
    do {
        if(txFn == NULL) {
            rval = -2;
            break;
        }

        if(len > MAX_PACKET_DATA_LEN) {
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

        txFn(
            header->len + sizeof(packet_footer_t) + sizeof(packet_header_t),
            packet_tx_buff);

    } while(0);


    return rval;
}
