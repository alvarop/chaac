#pragma once

// Note, if you update this, make sure to update the corresponding
// definitions in sw/chaac/chaac/packets.py
typedef enum {
    PACKET_TYPE_WEATHER_V1P0 = 10,
} packet_type_t;

typedef struct {
    uint32_t    uid;
    uint8_t     type;
} __attribute__ ((packed)) chaac_header_t;

typedef struct {
    chaac_header_t  header;
    uint32_t    sample;
    uint8_t     wind_dir;
    uint8_t     rain;
    uint16_t    rsvd;
    uint16_t    wind_speed;
    int16_t     temperature;
    uint16_t    humidity;
    int16_t     pressure;
    uint16_t    battery;
    uint16_t    solar_panel;
} __attribute__((packed)) weather_packet_v1p0_t;

typedef struct {
    int16_t rssi;
    int8_t snr;
    int8_t rsvd;
} __attribute__ ((packed)) chaac_lora_rxinfo_t;
