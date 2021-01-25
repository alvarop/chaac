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
#include "loraRadio.h"
#include "spi.h"


static void mainTask( void *parameters ) {
    (void)parameters;

    for(;;) {
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
        vTaskDelay(50);
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
        vTaskDelay(400);
    }
}

int main(void) {
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_USB_DEVICE_Init();
    MX_SPI1_Init();

    vcpInit();
    // vcpSetRxByteCallback(echo);

    loraRadioInit();

    BaseType_t rval = xTaskCreate(
            mainTask,
            "main",
            512,
            NULL,
            tskIDLE_PRIORITY + 1,
            NULL);

    configASSERT(rval == pdTRUE);

    vTaskStartScheduler();

    __builtin_unreachable();
}


