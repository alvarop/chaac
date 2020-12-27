#pragma once

#include "FreeRTOS.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    I2C_OK = 0,

    // For future use
    I2C_ANACK,
    I2C_DNACK,
    
    I2C_TIMEOUT,
    I2C_BUSY,
    I2C_ERR,
}I2CStatus_t;

BaseType_t xIOI2CInit(void *pvHandle);
BaseType_t xIOI2CDeInit(void *pvHandle);
bool bIOI2CProbe(void *pvHandle, uint8_t ucAddr);
I2CStatus_t xI2CTxRx(void *pvHandle, uint8_t ucAddr, void *vTxBuff, size_t xTxLen, void *vRxBuff, size_t xRxLen, uint32_t ulTimeout);

