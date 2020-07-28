#ifndef __BMP280_H__
#define __BMP280_H__

#include <stdint.h>

int32_t bmp280_init();
int32_t bmp280_start_forced_measurement();
int32_t bmp280_read(float *temperature, float *pressure);

#endif
