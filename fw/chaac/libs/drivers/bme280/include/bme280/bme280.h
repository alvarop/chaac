#ifndef __BME280_H__
#define __BME280_H__

#include <stdint.h>

int32_t bme280_init();
int32_t bme280_read(float *temperature, float *pressure, float *humidity);

#endif