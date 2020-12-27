#pragma once

#include <stdint.h>

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
    WIND_INVALID,
} wind_dir_t;

void vWindRainInit(); 
uint32_t ulWindRainGetRain();
void vWindRainClearRain();
uint32_t ulWindRainGetSpeed();
wind_dir_t xWindRainGetDir();
int16_t sWindRainGetDirDegrees();

void vWindRainRainIrq();
void vWindRainWindSpeedIrq();

