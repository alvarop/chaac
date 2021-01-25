#include "stm32l4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "debug.h"
#include "gpio.h"
#include "spi.h"
#include "radio.h"
#include "sx126x.h"
#include <string.h>
#include "sx126x-board.h"
#include "chaac_packet.h"
#include "packet.h"
#include "vcp.h"
#include "loraRadio.h"


#define RF_FREQUENCY 915000000

#define TX_OUTPUT_POWER 14

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8       // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0       // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            0
#define BUFFER_SIZE                                 64 // Define the payload size here

uint8_t txbuff[BUFFER_SIZE];

static RadioEvents_t RadioEvents;

void OnTxDone( void )
{
    Radio.Standby( );
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    const uint16_t total_size = size;// + sizeof(chaac_lora_rxinfo_t);
    if(total_size <= BUFFER_SIZE) {
        memcpy(txbuff, payload, size);

        chaac_lora_rxinfo_t *footer = (chaac_lora_rxinfo_t *)&txbuff[size];
        footer->rssi = rssi;
        footer->snr = snr;

        ulPacketTx(size + sizeof(chaac_lora_rxinfo_t), txbuff);

        Radio.Rx(RX_TIMEOUT_VALUE);
    }
}

void OnTxTimeout( void )
{
    Radio.Standby( );
}

void OnRxTimeout( void )
{
    /*Radio.Standby( );*/
    Radio.Rx( RX_TIMEOUT_VALUE );   //  Restart Rx
}

void OnRxError( void )
{
    /*Radio.Standby( );*/
    Radio.Rx( RX_TIMEOUT_VALUE );   //  Restart Rx
}

int init_radio(void) {

    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;
//    RadioEvents.CadDone = OnCadDone;

    SX126xIoInit();

    Radio.Init( &RadioEvents );

    // Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
    //                                LORA_SPREADING_FACTOR, LORA_CODINGRATE,
    //                                LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
    //                                true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );

       return 0;
}

void packetTxFn(int16_t len, void* data) {
    vcpTx(data, len);
}

TaskHandle_t pxRadioIrqTaskHandle = NULL;

static void prvRadioIrqTask( void *pvParameters ) {
    (void)pvParameters;

    init_radio();

    vPacketInitTxFn(packetTxFn);

    Radio.Rx(RX_TIMEOUT_VALUE);

    pxRadioIrqTaskHandle = xTaskGetCurrentTaskHandle();

    for(;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        Radio.IrqProcess();
    }
}

void loraRadioInit() {

    BaseType_t xRval = xTaskCreate(
            prvRadioIrqTask,
            "radio",
            512,
            NULL,
            tskIDLE_PRIORITY + 2,
            NULL);

    configASSERT(xRval == pdTRUE);

}
