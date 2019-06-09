#ifndef _WINDRAIN_H_
#define _WINDRAIN_H_

typedef enum {
    WIND_N      = 0,
    WIND_NNE    = 1,
    WIND_NE     = 2,
    WIND_ENE    = 3,
    WIND_E      = 4,
    WIND_ESE    = 5,
    WIND_SE     = 6,
    WIND_SSE    = 7,
    WIND_S      = 8,
    WIND_SSW    = 9,
    WIND_SW     = 10,
    WIND_WSW    = 11,
    WIND_W      = 12,
    WIND_WNW    = 13,
    WIND_NW     = 14,
    WIND_NNW    = 15,
} wind_dir_t;

void windrain_init();
uint32_t windrain_get_rain();
void windrain_clear_rain();
uint32_t windrain_get_speed();
wind_dir_t windrain_get_dir();
int16_t windrain_get_dir_degrees();

#endif /* _WINDRAIN_H_ */
