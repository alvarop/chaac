#pragma once

#include <stdint.h>

typedef enum {
    RADIO_MODE_STANDBY = 0,
    RADIO_MODE_SLEEP,
    RADIO_MODE_RX,
} loraMode_t;

typedef loraMode_t (*loraRxCb_t)(uint8_t *buff, size_t len, int16_t rssi, int8_t snr);
typedef loraMode_t (*loraNotifyCb_t)();

typedef struct {
    loraRxCb_t      rxCb;
    loraNotifyCb_t  txCb;
    loraNotifyCb_t  rxTimeoutCb;
    loraNotifyCb_t  txTimeoutCb;
    loraNotifyCb_t  rxErrorCb;
} loraRadioConfig_t;

void loraRadioInit(loraRadioConfig_t *config);
