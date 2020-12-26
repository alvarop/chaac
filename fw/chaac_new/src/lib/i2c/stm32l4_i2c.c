#include "io_i2c.h"
#include "stm32l4xx_hal.h"
#include "printf.h"

BaseType_t xIOI2CInit(void *pvHandle) {
    (void)pvHandle;
    return 0;
}

bool bIOI2CProbe(void *pvHandle, uint8_t ucAddr) {
    configASSERT(pvHandle != NULL);
    I2C_HandleTypeDef *pxHandle = (I2C_HandleTypeDef *)pvHandle;
    HAL_StatusTypeDef xStatus = HAL_I2C_IsDeviceReady(pxHandle, ucAddr, 1, 10);
    //printf("Trying %02X. Rval=%d\n", ucAddr, xStatus);    
    return (xStatus == HAL_OK);
}

// TODO - add mutex for multi-task use
// Rewrite using lower level functions to add repeated start and more descriptive errors
I2CStatus_t xI2CTxRx(void *pvHandle, uint8_t ucAddr, void *vTxBuff, size_t xTxLen, void *vRxBuff, size_t xRxLen, uint32_t ulTimeout) {
    configASSERT(pvHandle != NULL);
    I2C_HandleTypeDef *pxHandle = (I2C_HandleTypeDef *)pvHandle;
    I2CStatus_t xRval = I2C_OK;

    if(xTxLen && (vTxBuff != NULL)) {
        HAL_StatusTypeDef xHalRval = HAL_I2C_Master_Transmit(pxHandle, ucAddr, (uint8_t *)vTxBuff, xTxLen, ulTimeout);
        if (xHalRval == HAL_TIMEOUT) {
            xRval = I2C_TIMEOUT;
        } else if (xHalRval == HAL_BUSY) {
            xRval = I2C_BUSY;
        } else if (xHalRval != HAL_OK) {
            xRval = I2C_ERR;
        }
    }

    if((xRval == I2C_OK) && xRxLen && (vRxBuff != NULL)) {
        HAL_StatusTypeDef xHalRval = HAL_I2C_Master_Receive(pxHandle, ucAddr, (uint8_t *)vRxBuff, xRxLen, ulTimeout);
        if (xHalRval == HAL_TIMEOUT) {
            xRval = I2C_TIMEOUT;
        } else if (xHalRval == HAL_BUSY) {
            xRval = I2C_BUSY;
        } else if (xHalRval != HAL_OK) {
            xRval = I2C_ERR;
        }
    }

    return xRval;
}

