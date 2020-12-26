#include "stm32l4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "printf.h"
#include "io_i2c.h"
#include "debug.h"

extern I2C_HandleTypeDef hi2c1;

static void prvMainTask( void *pvParameters ) {
    (void)pvParameters;
    
    LL_GPIO_ResetOutputPin(RADIO_NRST_GPIO_Port, RADIO_NRST_Pin);

#ifdef BUILD_DEBUG
    vDebugInit();
    printf("Chaac FW\n");
#endif
    
    for(;;) {
        LL_GPIO_SetOutputPin(LED1_GPIO_Port, LED1_Pin);
        vTaskDelay(25);
        LL_GPIO_ResetOutputPin(LED1_GPIO_Port, LED1_Pin);
        vTaskDelay(25);
        LL_GPIO_SetOutputPin(RADIO_TXEN_GPIO_Port, RADIO_TXEN_Pin);
        vTaskDelay(25);
        LL_GPIO_ResetOutputPin(RADIO_TXEN_GPIO_Port, RADIO_TXEN_Pin);
        printf("I2C Scan\n");
        for(uint8_t ucAddr = 1; ucAddr < 128; ucAddr++) {
            if(bIOI2CProbe(&hi2c1, ucAddr << 1)) {
                printf("%02X Found!\n", ucAddr);
            }
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


