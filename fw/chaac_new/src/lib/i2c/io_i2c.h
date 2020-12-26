#pragma once

#include "FreeRTOS.h"
#include <stdbool.h>
#include <stdint.h>

BaseType_t xIOI2CInit(void *pvHandle);
bool bIOI2CProbe(void *pvHandle, uint8_t ucAddr);
 
