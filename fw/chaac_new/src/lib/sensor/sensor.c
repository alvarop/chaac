#include "FreeRTOS.h"
#include "task.h"
#include "sensor.h"

bool sensorGetAvg(sensor_t *sensor, float *avg) {
    configASSERT(sensor != NULL);
    configASSERT(avg != NULL);

    bool rval = false;

    taskENTER_CRITICAL();
    if (sensor->count > 0) {
        *avg = sensor->accumulator/(float)sensor->count;
        rval = true;
    }
    taskEXIT_CRITICAL();

    return rval;
}

void sensorAddSample(sensor_t *sensor, float val) {
    configASSERT(sensor != NULL);

    taskENTER_CRITICAL();
    sensor->accumulator += val;
    sensor->count++;
    taskEXIT_CRITICAL();
}


void sensorClearSamples(sensor_t *sensor) {
    configASSERT(sensor != NULL);

    taskENTER_CRITICAL();
    sensor->accumulator = 0;
    sensor->count = 0;
    taskEXIT_CRITICAL();
}