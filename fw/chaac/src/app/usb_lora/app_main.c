#include <stdbool.h>
#include "stm32l4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "printf.h"
#include "debug.h"
#include "gpio.h"
#include "usb_device.h"
#include "chaac_packet.h"
#include "packet.h"
#include "vcp.h"
#include "loraRadio.h"
#include "spi.h"

#define BUFFER_SIZE 64

static uint8_t txbuff[BUFFER_SIZE];

void vcpPacketTxFn(int16_t len, void* data) {
    vcpTx(data, len);
}

void radioPacketTxFn(int16_t len, void* data) {
    loraRadioSend(data, len);
}


loraMode_t loraRxCallback(uint8_t *buff, size_t len, int16_t rssi, int8_t snr){

    // Check packet CRC
    if(packetIsValid(buff, len) && (len <= BUFFER_SIZE)) {
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

loraMode_t loraTxCallback() {
    return RADIO_MODE_RX;
}

loraMode_t loraRxTimeoutCallback() {
    return RADIO_MODE_RX;
}

loraMode_t loraRxErrorCallback() {
    return RADIO_MODE_RX;
}


void packetRxFn(int16_t len, void* data) {
    if (len > 0) {
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
        packetTx(len, data, radioPacketTxFn);
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
    }
}

static loraRadioConfig_t loraConfig = {
    .startMode = RADIO_MODE_RX,
    .spiSetupFn = NULL,
    .spiTeardownFn = NULL,
    .rxCb = loraRxCallback,
    .txCb = loraTxCallback,
    .rxTimeoutCb = loraRxTimeoutCallback,
    .txTimeoutCb = loraRxErrorCallback,
    .rxErrorCb = NULL,
};

int main(void) {
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_USB_DEVICE_Init();
    MX_SPI1_Init();

    packetInitCb(packetRxFn);

    vcpInit();
    vcpSetRxByteCallback(packetProcessByte);

    loraRadioInit(&loraConfig);

    vTaskStartScheduler();

    __builtin_unreachable();
}

