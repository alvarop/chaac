#include "FreeRTOS.h"
#include "task.h"
#include "sht3x.h"
#include "IOI2c.h"
#include "printf.h"

#define RD_TH_HIGH_HOLD_H 0x2c
#define RD_TH_HIGH_HOLD_L 0x06
#define RD_TH_MED_HOLD_H 0x2c
#define RD_TH_MED_HOLD_L 0x0D
#define RD_TH_LOW_HOLD_H 0x2c
#define RD_TH_LOW_HOLD_L 0x10
#define RD_TH_HIGH_NOHOLD_H 0x24
#define RD_TH_HIGH_NOHOLD_L 0x00
#define RD_TH_MED_NOHOLD_H 0x24
#define RD_TH_MED_NOHOLD_L 0x0B
#define RD_TH_LOW_NOHOLD_H 0x24
#define RD_TH_LOW_NOHOLD_L 0x16
#define HEATER_ENABLE_H 0x30
#define HEATER_ENABLE_L 0x6D
#define HEATER_DISABLE_H 0x30
#define HEATER_DISABLE_L 0x66
#define RD_STATUS_H 0xF3
#define RD_STATUS_L 0x2D
#define CLR_STATUS_H 0x30
#define CLR_STATUS_L 0x41
#define RESET_H 0x30
#define RESET_L 0xA2


int32_t ulSht3xInit(void *pvI2CHandle, uint8_t ucAddr){
    configASSERT(pvI2CHandle != NULL);
	int32_t ulRval = 0;
    uint16_t usStatus = 0;
    do {
        ulRval = ulSht3xReset(pvI2CHandle, ucAddr);
        if(ulRval != 0) {
            printf("SHT could not reset\n");
            break;
        }

        // Device requires > 500us after soft reset
        vTaskDelay(2); 

        ulRval = ulSht3xStatus(pvI2CHandle, ucAddr, &usStatus);
        if(ulRval != 0) {
            printf("SHT could not read status\n");
            break;
        }

        if((usStatus & 0x10) == 0) {
            printf("SHT invalid status\n");
            ulRval = -1; // TODO
            break;
        }
    } while (0);


    return ulRval;
}

int32_t ulSht3xStatus(void *pvI2CHandle, uint8_t ucAddr, uint16_t *usStatus){
    configASSERT(pvI2CHandle != NULL);
    int32_t ulRval = 0;
    uint8_t pucRBuff[2];
    uint8_t pucWBuff[2] = {RD_STATUS_H, RD_STATUS_L};

    ulRval = xIOI2cTxRx(pvI2CHandle, ucAddr, pucWBuff, sizeof(pucWBuff), pucRBuff, sizeof(pucRBuff), 10);

    if((ulRval == I2C_OK) && (usStatus != NULL)) {
        *usStatus = (pucRBuff[0] << 8) | pucRBuff[1];
    }

    return ulRval;
}

int32_t ulSht3xReset(void *pvI2CHandle, uint8_t ucAddr){
    configASSERT(pvI2CHandle != NULL);
    int32_t ulRval = 0;
    uint8_t pcWBuff[2] = {RESET_H, RESET_L};

    ulRval = xIOI2cTxRx(pvI2CHandle, ucAddr, pcWBuff, sizeof(pcWBuff), NULL, 0, 10);

    return ulRval;
}

int32_t ulSht3xHeater(void *pvI2CHandle, uint8_t ucAddr, bool bEnable){
    int32_t ulRval = 0;
    uint8_t pcWBuff[2];

    if (bEnable) {
        pcWBuff[0] = HEATER_ENABLE_H;
        pcWBuff[1] = HEATER_ENABLE_L;
    } else {
        pcWBuff[0] = HEATER_DISABLE_H;
        pcWBuff[1] = HEATER_DISABLE_L;
    }

    ulRval = xIOI2cTxRx(pvI2CHandle, ucAddr, pcWBuff, sizeof(pcWBuff), NULL, 0, 10);

    return ulRval;
}

int32_t ulSht3xRead(void *pvI2CHandle, uint8_t ucAddr, int16_t *sTemperature, int16_t *sHumidity){
    int32_t ulRval = 0;

    uint8_t pucRBuff[6];
    uint8_t pucWBuff[2] = {RD_TH_HIGH_NOHOLD_H, RD_TH_HIGH_NOHOLD_L};

    do {
        // Send read command
        ulRval = xIOI2cTxRx(pvI2CHandle, ucAddr, pucWBuff, sizeof(pucWBuff), NULL, 0, 10);
        if(ulRval != 0) {
            break;
        }

        // Wait 20ms for sample
        vTaskDelay(pdMS_TO_TICKS(20));

        ulRval = xIOI2cTxRx(pvI2CHandle, ucAddr, NULL, 0, pucRBuff, sizeof(pucRBuff), 10);

        if(ulRval != 0) {
            break;
        }

        if(sTemperature != NULL) {
            uint32_t ulTemp;
            // Get sTemperature * 100
            ulTemp = (pucRBuff[0] << 8) | pucRBuff[1];
            ulTemp = -4500 + (ulTemp * 17500)/0xFFFF;
            *sTemperature = (uint16_t)ulTemp;
        }

        if(sHumidity != NULL) {
            uint32_t ulRh;
            // Get sHumidity * 100
            ulRh = (pucRBuff[3] << 8) | pucRBuff[4];
            ulRh = (10000 * ulRh)/0xFFFF;
            *sHumidity = (uint16_t)ulRh;
        }

    } while(0);


    return ulRval;
}

 
