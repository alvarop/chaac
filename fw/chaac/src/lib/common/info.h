#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t getHWID();
bool getHWIDStr(char *buff, size_t len);
const char *getFWVersionStr();
const char *getHWVersionStr();
const char *getFWTypeStr();

#ifdef __cplusplus
}
#endif
