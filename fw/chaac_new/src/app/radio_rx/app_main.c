#include "stm32l4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "printf.h"
#include "debug.h"
#include "gpio.h"
#include "usart.h"

static void prvMainTask( void *pvParameters ) {
    (void)pvParameters;

#ifdef BUILD_DEBUG
    vDebugInit();
    // printf("Chaac FW\n");
#endif

    for(;;) {
        LL_GPIO_ResetOutputPin(LD2_GPIO_Port, LD2_Pin);
        vTaskDelay(25);
        LL_GPIO_SetOutputPin(LD2_GPIO_Port, LD2_Pin);

        vTaskDelay(925);
    }
}

int main(void) {
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    // MX_SPI1_Init();
    MX_USART2_UART_Init();

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


