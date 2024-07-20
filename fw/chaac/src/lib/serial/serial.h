#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef void (*serialRxByteCallback_t)(uint8_t byte);

void serialInit(void *dev);
size_t serialGetTxBytes(void *buff, size_t size);
size_t serialGetTxBytesFromISR(void *buff, size_t size);
bool serialRxBytesFromISR(void *buff, size_t len);
void serialTx(void *buff, size_t len);
void serialIRQHandler();

void serialSetRxByteCallback(serialRxByteCallback_t callback);
