#include <stdbool.h>
#include "stm32l4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "printf.h"
#include "debug.h"
#include "gpio.h"
#include "usb_device.h"
#include "vcp.h"

// #include "spi.h"
// #include "usart.h"


static void prvMainTask( void *pvParameters ) {
    (void)pvParameters;

    for(;;) {
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
        vTaskDelay(50);
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
        vTaskDelay(400);

        HAL_GPIO_WritePin(RADIO_TXEN_GPIO_Port, RADIO_TXEN_Pin, GPIO_PIN_SET);
        vTaskDelay(50);
        HAL_GPIO_WritePin(RADIO_TXEN_GPIO_Port, RADIO_TXEN_Pin, GPIO_PIN_RESET);
        vTaskDelay(400);
        vcpSendBytes("blink!!!\n", 9);
    }
}

int main(void) {
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_USB_DEVICE_Init();
    // MX_SPI1_Init();
    // MX_USART2_UART_Init();

    vcpInit();

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


