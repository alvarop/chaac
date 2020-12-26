#include "stm32l4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "printf.h"
#include "io_i2c.h"
#include "debug.h"
#include "sht3x.h"
#include "dps368.h"

extern I2C_HandleTypeDef hi2c1;

static void prvMainTask( void *pvParameters ) {
    (void)pvParameters;
    
    LL_GPIO_ResetOutputPin(RADIO_NRST_GPIO_Port, RADIO_NRST_Pin);

#ifdef BUILD_DEBUG
    vDebugInit();
    printf("Chaac FW\n");
#endif
   
    uint32_t ulRval = ulSht3xInit(&hi2c1, SHT3x_ADDR);
    if(ulRval == 0) {
        printf("SHT3x Initialized Successfully!\n");
    } else {
        printf("Error initializing SHT3x (%ld)\n", ulRval);
    }

    ulRval = dps368_init(&hi2c1);
    if(ulRval == 0) {
        printf("DPS368 Initialized Successfully!\n");
    } else {
        printf("Error initializing DPS368 (%ld)\n", ulRval);
    }

    for(;;) {
        LL_GPIO_SetOutputPin(LED1_GPIO_Port, LED1_Pin);
        vTaskDelay(25);
        LL_GPIO_ResetOutputPin(LED1_GPIO_Port, LED1_Pin);
        vTaskDelay(25);
        LL_GPIO_SetOutputPin(RADIO_TXEN_GPIO_Port, RADIO_TXEN_Pin);
        vTaskDelay(25);
        LL_GPIO_ResetOutputPin(RADIO_TXEN_GPIO_Port, RADIO_TXEN_Pin);
        /*printf("I2C Scan\n");*/
        /*for(uint8_t ucAddr = 1; ucAddr < 128; ucAddr++) {*/
            /*if(bIOI2CProbe(&hi2c1, ucAddr << 1)) {*/
                /*printf("%02X Found!\n", ucAddr);*/
            /*}*/
        /*}*/

        int16_t sTemperature, sHumidity;
        ulRval = ulSht3xRead(&hi2c1, SHT3x_ADDR, &sTemperature, &sHumidity);
        if(ulRval == 0) {
            printf("T: %0.2fC H: %0.2f %RH\n", (float)sTemperature/100.0, (float)sHumidity/100.0);
        } else {
            printf("Error reading from SHT3x (%ld)\n", ulRval);
        }

        float fTemperature, fPressure;
 
        ulRval = dps368_measure_temp_once(&fTemperature);
        if (ulRval != 0) {
            printf("Error reading DPS368 Temperature (%d)\n", ulRval);
        }
        ulRval = dps368_measure_pressure_once(&fPressure);
        if (ulRval != 0) {
            printf("Error reading DPS368 Pressure (%d)\n", ulRval);
        }
 
        if (ulRval == 0) {
            printf("T: %0.2f C T: %0.2f hPa\n", fTemperature, fPressure/100.0);
        }



        vTaskDelay(4925);
    }
}

__attribute__((noreturn)) void app_main() {
    BaseType_t xRval = xTaskCreate(
            prvMainTask,
            "main",
            512,
            NULL,
            tskIDLE_PRIORITY + 1,
            NULL);

    configASSERT(xRval == pdTRUE);

    vTaskStartScheduler();

    __builtin_unreachable();
}


