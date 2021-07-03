#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float accumulator;
    uint32_t count;
} sensor_t;

bool sensorGetAvg(sensor_t *sensor, float *avg);
void sensorAddSample(sensor_t *sensor, float val);
void sensorClearSamples(sensor_t *sensor);
