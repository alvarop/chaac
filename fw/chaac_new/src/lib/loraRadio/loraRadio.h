#pragma once

#include <stdint.h>

typedef enum {
    RADIO_MODE_STANDBY = 0,
    RADIO_MODE_SLEEP,
    RADIO_MODE_RX,
} loraMode_t;

typedef loraMode_t (*loraRxCallback_t)(uint8_t *buff, size_t len, int16_t rssi, int8_t snr);
typedef loraMode_t (*loraTxCallback_t)();

void loraRadioInit(loraRxCallback_t rxCallback, loraTxCallback_t txCallback);
