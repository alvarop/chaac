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

typedef struct {
    int32_t (*getDirMvFn)(); // Function to read direction voltage
} windDirCfg_t;

void windRainInit(windDirCfg_t *cfg);
uint32_t windRainGetRain();
void windRainClearRain();
uint32_t windRainGetSpeed();
uint32_t windRainGetGust();
wind_dir_t windRainGetDir();
int16_t windRainGetAvgDirDegrees();
int16_t windRainGetDirDegrees();

void windRainRainIrq();
void windRainWindSpeedIrq();

