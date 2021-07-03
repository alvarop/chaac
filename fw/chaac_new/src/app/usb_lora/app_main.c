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
void loraRxCallback(uint8_t *buff, size_t len, int16_t rssi, int8_t snr){
    const uint16_t total_size = len;// + sizeof(chaac_lora_rxinfo_t);
    if(total_size <= BUFFER_SIZE) {
        memcpy(txbuff, buff, len);

        chaac_lora_rxinfo_t *footer = (chaac_lora_rxinfo_t *)&txbuff[len];
        footer->rssi = rssi;
        footer->snr = snr;

        ulPacketTx(len + sizeof(chaac_lora_rxinfo_t), txbuff);

    }

    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    vTaskDelay(50);
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
}

void packetTxFn(int16_t len, void* data) {
    vcpTx(data, len);
}

int main(void) {
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_USB_DEVICE_Init();
    MX_SPI1_Init();

    vcpInit();
    // vcpSetRxByteCallback(echo);

    vPacketInitTxFn(packetTxFn);
    loraRadioInit(loraRxCallback);

    vTaskStartScheduler();

    __builtin_unreachable();
}

