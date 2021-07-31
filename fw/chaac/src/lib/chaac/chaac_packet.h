#pragma once

// Note, if you update this, make sure to update the corresponding
// definitions in sw/chaac/chaac/packets.py
typedef enum {
    PACKET_TYPE_RESET = 1,
    PACKET_TYPE_MEMFAULT = 2,
    PACKET_TYPE_WEATHER_V1P0 = 10,
    PACKET_TYPE_WEATHER_V1P1 = 11,
    PACKET_TYPE_USB_CMD = 0xE0,
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
    chaac_header_t  header;
    uint32_t    sample;
    uint16_t    wind_dir_deg;
    uint8_t     rain;
    uint16_t    wind_speed;
    uint16_t    gust_speed;
    int16_t     temperature;
    int16_t     alt_temperature;
    uint16_t    humidity;
    int16_t     pressure;
    uint16_t    battery;
    uint16_t    solar_panel;
} __attribute__((packed)) weather_packet_v1p1_t;


typedef struct {
    int16_t rssi;
    int8_t snr;
    int8_t rsvd;
} __attribute__ ((packed)) chaac_lora_rxinfo_t;

typedef struct {
    chaac_header_t  header;
    uint16_t    len;
    uint8_t     buf[0];
} __attribute__((packed)) memfault_packet_t;

typedef struct {
    chaac_header_t  header;
    uint16_t        cmd;
    uint16_t        len;
    uint8_t         buf[0];
} __attribute__((packed)) usb_cmd_packet_t;
