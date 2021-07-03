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

int main(void) {
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_USB_DEVICE_Init();
    MX_SPI1_Init();

    vcpInit();
    // vcpSetRxByteCallback(echo);

    loraRadioInit();

    vTaskStartScheduler();

    __builtin_unreachable();
}

