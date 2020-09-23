#ifndef __CHAAC_PACKET_H__
#define __CHAAC_PACKET_H__

// Note, if you update this, make sure to update the corresponding
// definitions in sw/chaac/chaac/packets.py
typedef enum {
    PACKET_TYPE_BUTTON_V1P0 = 30,
} packet_type_t;

typedef struct {
    uint32_t    uid;
    uint8_t     type;
} __attribute__ ((packed)) chaac_header_t;

typedef struct {
    chaac_header_t  header;
    uint8_t     ch1;
    uint8_t     ch2;
    uint16_t    battery;
} __attribute__((packed)) button_packet_v1p0_t;


#endif
