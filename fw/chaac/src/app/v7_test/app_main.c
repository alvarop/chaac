#include <stdbool.h>

#include "FreeRTOS.h"
#include "chaac_packet.h"
#include "debug.h"
#include "gpio.h"
#include "info.h"
#include "loraRadio.h"
#include "main.h"
#include "memfault/components.h"
#include "memfault/core/data_packetizer.h"
#include "packet.h"
#include "printf.h"
#include "serial.h"
#include "spi.h"
#include "stm32l4xx_hal.h"
#include "task.h"
#include "usart.h"
#include "usb_device.h"
#include "usb_dfu.h"
#include "vcp.h"

#define BUFFER_SIZE 300

static uint8_t txbuff[BUFFER_SIZE];

void vcpPacketTxFn(uint16_t len, void *data) { vcpTx(data, len); }

void radioPacketTxFn(uint16_t len, void *data) { loraRadioSend(data, len); }

loraMode_t loraRxCallback(uint8_t *buff, size_t len, int16_t rssi, int8_t snr) {
  // Check packet CRC
  if (packetIsValid(buff, len) && (len <= BUFFER_SIZE)) {
    packet_header_t *header = (packet_header_t *)buff;

    memcpy(txbuff, (void *)&header[1], header->len);

    chaac_lora_rxinfo_t *footer = (chaac_lora_rxinfo_t *)&txbuff[len];
    footer->rssi = rssi;
    footer->snr = snr;

    packetTx(len + sizeof(chaac_lora_rxinfo_t), txbuff, vcpPacketTxFn);
  }

  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
  vTaskDelay(50);
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

  return RADIO_MODE_RX;
}

loraMode_t loraTxCallback() { return RADIO_MODE_RX; }

loraMode_t loraRxTimeoutCallback() { return RADIO_MODE_RX; }

loraMode_t loraTxTimeoutCallback() { return RADIO_MODE_RX; }

loraMode_t loraRxErrorCallback() { return RADIO_MODE_RX; }

typedef enum {
  usbCmdReboot = 0,
  usbCmdBootloader = 1,
  getMemfaultData = 2,
  usbCmdPing = 3,
  usbCmdPong = 4,
} chaacUSBCommand_t;

#define MEMFAULT_DATA_MAX_LEN 256
static void sendMemfaultData() {
  uint8_t *buf = pvPortMalloc(MEMFAULT_DATA_MAX_LEN);
  memfault_packet_t *packet = (memfault_packet_t *)buf;
  packet->header.uid = getHWID();
  packet->header.type = PACKET_TYPE_MEMFAULT;
  size_t buf_len = MEMFAULT_DATA_MAX_LEN - sizeof(memfault_packet_t);
  if (memfault_packetizer_get_chunk(&packet[1], &buf_len)) {
    packet->len = buf_len;
    packetTx(buf_len + sizeof(memfault_packet_t), buf, vcpPacketTxFn);
  }
  vPortFree(buf);
}

static loraRadioConfig_t loraConfig = {
    .startMode = RADIO_MODE_RX,
    .spiSetupFn = NULL,
    .spiTeardownFn = NULL,
    .rxCb = loraRxCallback,
    .txCb = loraTxCallback,
    .rxTimeoutCb = loraRxTimeoutCallback,
    .txTimeoutCb = loraTxTimeoutCallback,
    .rxErrorCb = loraRxErrorCallback,
};

void serialProcessByte(uint8_t byte) {
  vcpTx(&byte, 1);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
  vTaskDelay(5);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
}

void usbProcessByte(uint8_t byte) {
  serialTx(&byte, 1);
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
  vTaskDelay(5);
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
}

int main(void) {
  dfuCheck();

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
  MX_LPUART1_UART_Init();
  MX_SPI1_Init();

  memfault_platform_boot();

  // packetInitCb(packetRxFn);

  vcpInit();
  vcpSetRxByteCallback(usbProcessByte);

  serialInit(LPUART1);
  serialSetRxByteCallback(serialProcessByte);

  // loraRadioInit(&loraConfig);

  vTaskStartScheduler();

  __builtin_unreachable();
}
