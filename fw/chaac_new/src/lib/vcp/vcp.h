#pragma once

#include <stdbool.h>

typedef void (*rxByteCallback_t)(uint8_t byte);

void vcpInit();
size_t vcpGetTxBytes(void *buff, size_t size);
size_t vcpGetTxBytesFromISR(void *buff, size_t size);
void vcpRxBytesFromISR(void *buff, size_t len);
void vcpTx(void *buff, size_t len);
void vcpSetConnectedState(bool state);

void vcpSetRxByteCallback(rxByteCallback_t callback);
