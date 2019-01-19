#include <stdio.h>
#include <packet/packet.h>
#include <packet/crc.h>
#include <string.h>

static uint8_t buff[MAX_PACKET_SIZE];
static uint32_t buff_index = 0;
static packet_header_t *header = (packet_header_t*)(buff);

typedef enum {
    PACKET_STATE_CLEAR,
    PACKET_STATE_START,
    PACKET_STATE_LEN,
    PACKET_STATE_DATA,
    PACKET_STATE_CRC
} packet_state_t;

static packet_state_t state = PACKET_STATE_CLEAR;

static void (*packet_cb_fn)(int16_t len, void *data) = NULL;

int32_t packet_init_cb(void (*cb_fn)(int16_t len, void *data)) {
    packet_cb_fn = cb_fn;

    return 0;
}

int32_t packet_process_byte(uint8_t byte) {
    switch(state) {
        case PACKET_STATE_CLEAR: {
            if (byte == (PACKET_START & 0xFF)) {
                buff[buff_index++] = byte;
                state = PACKET_STATE_START;
            } else {
                buff_index = 0;
                state = PACKET_STATE_CLEAR;
            }
            break;
        }

        case PACKET_STATE_START: {
            if (byte == ((PACKET_START >> 8) & 0xFF)) {
                buff[buff_index++] = byte;
                state = PACKET_STATE_LEN;
            } else {
                buff_index = 0;
                state = PACKET_STATE_CLEAR;
            }
            break;
        }

        case PACKET_STATE_LEN: {
            buff[buff_index++] = byte;

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
            buff[buff_index++] = byte;

            if (buff_index < (sizeof(packet_header_t) + header->len)) {
                // Need more data bytes
                break;
            }

            state = PACKET_STATE_CRC;
            break;
        }

        case PACKET_STATE_CRC: {
            buff[buff_index++] = byte;

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

    return 0;
}
