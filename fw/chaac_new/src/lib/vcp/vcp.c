#include <stdbool.h>
#include <string.h>
#include "stm32l4xx_hal.h"
#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "usbd_cdc_if.h"
#include "queue.h"
#include "vcp.h"

static StreamBufferHandle_t vcpTxStreamBuffer;
static StreamBufferHandle_t vcpRxStreamBuffer;
static volatile bool vcpConnected = false;

static rxByteCallback_t processByteCallback = NULL;

#define TX_QUEUE_SIZE 8
static QueueHandle_t vcpTxQueue = NULL;

typedef struct {
    void *buff;
    size_t len;
} SizedBuff_t;

size_t vcpGetTxBytes(void *buff, size_t size) {
    configASSERT(buff != NULL);

    return xStreamBufferReceive(vcpTxStreamBuffer, buff, size, 0);
}

size_t vcpGetTxBytesFromISR(void *buff, size_t size) {
    configASSERT(buff != NULL);

    return xStreamBufferReceiveFromISR(vcpTxStreamBuffer, buff, size, NULL);
}

void vcpRxBytesFromISR(void *buff, size_t len) {
    configASSERT(buff != NULL);
    size_t bytesSent = xStreamBufferSendFromISR(vcpRxStreamBuffer, buff, len, NULL);
    if(bytesSent != len) {
        // buffer is full
        // TODO - do something about it
    }
}

void vcpSetConnectedState(bool state) {
    if(state) {
        vcpConnected = true;
    } else {
        vcpConnected = false;
    }
}

void vcpTx(void *buff, size_t len) {
    configASSERT(buff != NULL);
    SizedBuff_t sbuff = {NULL , 0};
    sbuff.buff = pvPortMalloc(len);
    configASSERT(sbuff.buff != NULL);

    memcpy(sbuff.buff, buff, len);
    sbuff.len = len;

    // Buffer will be freed after use in vcpTxTask
    if(xQueueSend(vcpTxQueue, &sbuff, 10) != pdTRUE) {
        // Unable to send message, free buffer here
        vPortFree(sbuff.buff);
    }
}

void vcpSetRxByteCallback(rxByteCallback_t callback) {
    processByteCallback = callback;
}

static void vcpTxBuff(void *buff, size_t len) {
    if(vcpConnected) {
        size_t bytesSent = xStreamBufferSend(vcpTxStreamBuffer, buff, len, 50 );

        configASSERT(bytesSent == len);

        // Start/continue usb transfer
        uint8_t result = CDC_Transmit_FS(NULL, 0);
        configASSERT(result != USBD_FAIL);
    }
}

static void vcpTxTask( void *parameters ) {
    (void)parameters;

    for(;;) {
        SizedBuff_t sbuff;
        BaseType_t rval = xQueueReceive(vcpTxQueue, &sbuff, portMAX_DELAY);
        configASSERT(rval == pdTRUE);

        configASSERT(sbuff.buff != NULL);

        vcpTxBuff(sbuff.buff, sbuff.len);

        // Done using the buffer
        vPortFree(sbuff.buff);
    }
}

static void vcpRxTask( void *parameters ) {
    (void)parameters;

    for(;;) {
        uint8_t rxByte;
        size_t rxLen = xStreamBufferReceive(vcpRxStreamBuffer,
                                            &rxByte,
                                            sizeof(rxByte),
                                            portMAX_DELAY);
        if(rxLen && (processByteCallback != NULL)) {
            processByteCallback(rxByte);
        }
    }
}

void vcpInit() {
    vcpTxStreamBuffer = xStreamBufferCreate(128, 1);
    configASSERT(vcpTxStreamBuffer != NULL);

    vcpRxStreamBuffer = xStreamBufferCreate(128, 1);
    configASSERT(vcpRxStreamBuffer != NULL);

    vcpTxQueue = xQueueCreate(TX_QUEUE_SIZE, sizeof(SizedBuff_t));
    configASSERT(vcpTxQueue != NULL);

    BaseType_t rval = xTaskCreate(
        vcpTxTask,
        "main",
        256,
        NULL,
        tskIDLE_PRIORITY + 2,
        NULL);

    configASSERT(rval == pdTRUE);


    rval = xTaskCreate(
        vcpRxTask,
        "main",
        256,
        NULL,
        tskIDLE_PRIORITY + 2,
        NULL);

    configASSERT(rval == pdTRUE);
}
