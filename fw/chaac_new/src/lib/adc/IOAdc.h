#pragma once

#include "FreeRTOS.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    ADC_OK = 0,
    ADC_ERR,
}AdcStatus_t;

BaseType_t xIOAdcInit(void *pvHandle);
BaseType_t xIOAdcDeInit(void *pvHandle);

AdcStatus_t xIOAdcRead(void *pvHandle, int32_t *plValue); 
AdcStatus_t xIOAdcConfig(void *pvHandle, void *pvConfig);
AdcStatus_t xIOAdcChannelConfig(void *pvHandle, void *pvChannelConfig);

