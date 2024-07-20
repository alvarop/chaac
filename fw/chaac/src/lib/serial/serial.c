#include "serial.h"

#include <stdbool.h>
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "serial.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_ll_lpuart.h"
#include "stream_buffer.h"

static StreamBufferHandle_t serialTxStreamBuffer;
static StreamBufferHandle_t serialRxStreamBuffer;

static serialRxByteCallback_t processByteCallback = NULL;

#define TX_QUEUE_SIZE 8
static QueueHandle_t serialTxQueue = NULL;
static USART_TypeDef *device = NULL;

typedef struct {
  void *buff;
  size_t len;
} SizedBuff_t;

static void serialTxBuff(void *buff, size_t len);

size_t serialGetTxBytes(void *buff, size_t size) {
  configASSERT(buff != NULL);

  return xStreamBufferReceive(serialTxStreamBuffer, buff, size, 0);
}

size_t serialGetTxBytesFromISR(void *buff, size_t size) {
  configASSERT(buff != NULL);

  return xStreamBufferReceiveFromISR(serialTxStreamBuffer, buff, size, NULL);
}

bool serialRxBytesFromISR(void *buff, size_t len) {
  configASSERT(buff != NULL);
  size_t bytesSent =
      xStreamBufferSendFromISR(serialRxStreamBuffer, buff, len, NULL);
  if (bytesSent != len) {
    // buffer is full
    // TODO - do something about it
    return pdFALSE;
  } else {
    return pdTRUE;
  }
}

void serialTx(void *buff, size_t len) {
  configASSERT(buff != NULL);

  serialTxBuff(buff, len);
}

void serialSetRxByteCallback(serialRxByteCallback_t callback) {
  processByteCallback = callback;
}

static void serialTxBuff(void *buff, size_t len) {
  size_t bytesSent = xStreamBufferSend(serialTxStreamBuffer, buff, len, 50);

  configASSERT(bytesSent == len);

  if (!LL_LPUART_IsEnabledIT_TXE((USART_TypeDef *)device)) {
    LL_LPUART_EnableIT_TXE((USART_TypeDef *)device);
    LL_LPUART_EnableIT_TC((USART_TypeDef *)device);
  }
}

static void serialRxTask(void *parameters) {
  (void)parameters;

  for (;;) {
    uint8_t rxByte;
    size_t rxLen = xStreamBufferReceive(serialRxStreamBuffer, &rxByte,
                                        sizeof(rxByte), portMAX_DELAY);
    if (rxLen && (processByteCallback != NULL)) {
      processByteCallback(rxByte);
    }
  }
}

void serialIRQHandler() {
  BaseType_t higherPriorityTaskWoken = pdFALSE;
  size_t bytesAvailable = 0;
  // Process received bytes
  if (LL_LPUART_IsActiveFlag_RXNE((USART_TypeDef *)device) &&
      LL_LPUART_IsEnabledIT_RXNE((USART_TypeDef *)device)) {
    uint8_t byte = LL_LPUART_ReceiveData8((USART_TypeDef *)device);
    higherPriorityTaskWoken = serialRxBytesFromISR(&byte, 1);
  }

  // Process bytes to transmit
  if (LL_LPUART_IsActiveFlag_TXE((USART_TypeDef *)device) &&
      LL_LPUART_IsEnabledIT_TXE((USART_TypeDef *)device)) {
    uint8_t txByte;
    bytesAvailable = serialGetTxBytesFromISR(&txByte, 1);

    if (bytesAvailable > 0) {
      // Transmit current byte
      LL_LPUART_TransmitData8((USART_TypeDef *)device, txByte);
    } else {
      // Disable this interrupt if there are no more bytes to transmit
      LL_LPUART_DisableIT_TXE((USART_TypeDef *)device);
    }
  }

  if (!bytesAvailable && LL_LPUART_IsActiveFlag_TC((USART_TypeDef *)device) &&
      !LL_LPUART_IsEnabledIT_TXE((USART_TypeDef *)device)) {
    // Done transmitting
    LL_LPUART_ClearFlag_TC((USART_TypeDef *)device);
  }

  // Handle UART overrun error
  if (LL_LPUART_IsActiveFlag_ORE((USART_TypeDef *)device)) {
    // TODO - handle it
    LL_LPUART_ClearFlag_ORE((USART_TypeDef *)device);
  }

  portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

void serialInit(void *dev) {
  configASSERT(dev);
  device = (USART_TypeDef *)dev;

  serialTxStreamBuffer = xStreamBufferCreate(256, 1);
  configASSERT(serialTxStreamBuffer != NULL);

  serialRxStreamBuffer = xStreamBufferCreate(256, 1);
  configASSERT(serialRxStreamBuffer != NULL);

  serialTxQueue = xQueueCreate(TX_QUEUE_SIZE, sizeof(SizedBuff_t));
  configASSERT(serialTxQueue != NULL);

  BaseType_t rval = xTaskCreate(serialRxTask, "serial_rx", 256, NULL,
                                tskIDLE_PRIORITY + 2, NULL);

  configASSERT(rval == pdTRUE);
}
