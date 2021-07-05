#pragma once

#include <stdint.h>

typedef void (*loraRxCallback_t)(uint8_t *buff, size_t len, int16_t rssi, int8_t snr);

void loraRadioInit(loraRxCallback_t rxCallback);
