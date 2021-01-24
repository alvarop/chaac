#include <stdbool.h>
#include "stm32l4xx_hal.h"
#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "task.h"
#include "main.h"
#include "printf.h"
#include "debug.h"
#include "gpio.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
// #include "spi.h"
// #include "usart.h"

StreamBufferHandle_t cdcTxStreamBuffer;
StreamBufferHandle_t cdcRxStreamBuffer;
volatile bool vcpConnected = false;

void sendBytes(void *buff, size_t len);

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
        sendBytes("blink...\n", 9);
    }
}

size_t vcpGetTxBytes(void *buff, size_t size) {
    configASSERT(buff != NULL);

    return xStreamBufferReceive(cdcTxStreamBuffer, buff, size, 0);
}

size_t vcpGetTxBytesFromISR(void *buff, size_t size) {
    configASSERT(buff != NULL);

    return xStreamBufferReceiveFromISR(cdcTxStreamBuffer, buff, size, NULL);
}

void vcpConnectedState(uint8_t state) {
    if(state) {
        vcpConnected = true;
    } else {
        vcpConnected = false;
    }
}

void sendBytes(void *buff, size_t len) {
    if(vcpConnected) {
        size_t bytesSent = xStreamBufferSend(cdcTxStreamBuffer, buff, len, 50 );

        configASSERT(bytesSent == len);

        // Start/continue usb transfer
        uint8_t result = CDC_Transmit_FS(NULL, 0);
        configASSERT(result != USBD_FAIL);
    }
}

int main(void) {
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_USB_DEVICE_Init();
    // MX_SPI1_Init();
    // MX_USART2_UART_Init();

    cdcTxStreamBuffer = xStreamBufferCreate(128, 1);
    configASSERT(cdcTxStreamBuffer != NULL);

    cdcRxStreamBuffer = xStreamBufferCreate(128, 1);
    configASSERT(cdcRxStreamBuffer != NULL);

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


