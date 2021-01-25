#include <stdbool.h>
#include "stm32l4xx_hal.h"
#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "usbd_cdc_if.h"

static StreamBufferHandle_t cdcTxStreamBuffer;
static StreamBufferHandle_t cdcRxStreamBuffer;
static volatile bool vcpConnected = false;


size_t vcpGetTxBytes(void *buff, size_t size) {
    configASSERT(buff != NULL);

    return xStreamBufferReceive(cdcTxStreamBuffer, buff, size, 0);
}

size_t vcpGetTxBytesFromISR(void *buff, size_t size) {
    configASSERT(buff != NULL);

    return xStreamBufferReceiveFromISR(cdcTxStreamBuffer, buff, size, NULL);
}

void vcpSetConnectedState(bool state) {
    if(state) {
        vcpConnected = true;
    } else {
        vcpConnected = false;
    }
}

void vcpSendBytes(void *buff, size_t len) {
    if(vcpConnected) {
        size_t bytesSent = xStreamBufferSend(cdcTxStreamBuffer, buff, len, 50 );

        configASSERT(bytesSent == len);

        // Start/continue usb transfer
        uint8_t result = CDC_Transmit_FS(NULL, 0);
        configASSERT(result != USBD_FAIL);
    }
}

void vcpInit() {
	cdcTxStreamBuffer = xStreamBufferCreate(128, 1);
    configASSERT(cdcTxStreamBuffer != NULL);

    cdcRxStreamBuffer = xStreamBufferCreate(128, 1);
    configASSERT(cdcRxStreamBuffer != NULL);
}
