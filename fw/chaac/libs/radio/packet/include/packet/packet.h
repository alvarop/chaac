#ifndef __PACKET_H__
#define __PACKET_H__

#include <stdint.h>

typedef enum {
    PACKET_TYPE_DATA = 1,
    PACKET_TYPE_GPS = 2,
    PACKET_TYPE_CMD = 3,
} packet_type_t;

typedef struct {
    uint32_t    uid;
    uint8_t     type;
} __attribute__ ((packed)) chaac_header_t;

typedef struct {
    chaac_header_t  header;
    float           wind_speed;
    float           wind_dir;
    float           rain;
    float           temperature;
    float           humidity;
    float           temperature_in;
    float           pressure;
    float           light;
    float           battery;
} __attribute__((packed)) weather_data_packet_t;

// TODO
typedef struct {
    chaac_header_t  header;
    int32_t         lat_degrees;
    double          lat_minutes;
    char            lat_cardinal;
    int32_t         lon_degrees;
    double          lon_minutes;
    char            lon_cardinal;
} __attribute__((packed)) weather_gps_packet_t;

typedef struct {
    chaac_header_t  header;
    uint8_t         cmd;
} __attribute__((packed)) cmd_packet_t;

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
int32_t packet_process_byte(uint8_t byte);

#endif
