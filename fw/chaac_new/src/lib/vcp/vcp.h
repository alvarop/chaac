#pragma once

#include <stdbool.h>

void vcpInit();
size_t vcpGetTxBytes(void *buff, size_t size);
size_t vcpGetTxBytesFromISR(void *buff, size_t size);
void vcpSendBytes(void *buff, size_t len);
void vcpSetConnectedState(bool state);
