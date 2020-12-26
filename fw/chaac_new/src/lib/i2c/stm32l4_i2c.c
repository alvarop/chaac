#include "io_i2c.h"
#include "stm32l4xx_hal.h"
#include "printf.h"

/*HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size,*/
                                          /*uint32_t Timeout);*/
/*HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size,*/
                                         /*uint32_t Timeout);*/
/*HAL_StatusTypeDef HAL_I2C_Slave_Transmit(I2C_HandleTypeDef *hi2c, uint8_t *pData, uint16_t Size, uint32_t Timeout);*/
/*HAL_StatusTypeDef HAL_I2C_Slave_Receive(I2C_HandleTypeDef *hi2c, uint8_t *pData, uint16_t Size, uint32_t Timeout);*/
/*HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress,*/
                                    /*uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);*/
/*HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress,*/
                                   /*uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);*/
/*HAL_StatusTypeDef HAL_I2C_IsDeviceReady(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Trials,*/
                                        /*uint32_t Timeout);*/

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

