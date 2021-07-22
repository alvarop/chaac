#pragma once

#include <stdint.h>
#include <stdbool.h>

#define SHT3x_ADDR (0x44 << 1)
#define SHT3x_ALT_ADDR (0x45 << 1)

int32_t ulSht3xInit(void *pvI2CHandle, uint8_t ucAddr);
int32_t ulSht3xStatus(void *pvI2CHandle, uint8_t ucAddr, uint16_t *usStatus);
int32_t ulSht3xReset(void *pvI2CHandle, uint8_t ucAddr);
int32_t ulSht3xHeater(void *pvI2CHandle, uint8_t ucAddr, bool bEnable);
int32_t ulSht3xRead(void *pvI2CHandle, uint8_t ucAddr, int16_t *sTemperature, int16_t *sHumidity);
            
