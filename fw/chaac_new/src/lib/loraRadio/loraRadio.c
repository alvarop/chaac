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

static RadioEvents_t RadioEvents;
static loraRadioConfig_t *_config = NULL;
static uint32_t _rxTimeout = 0;

void radioEnterMode(loraMode_t mode) {
    switch(mode) {
        case RADIO_MODE_STANDBY: {
            Radio.Standby();
            break;
        }
        case RADIO_MODE_SLEEP: {
            Radio.Sleep();
            break;
        }
        case RADIO_MODE_RX: {
            Radio.Rx(_rxTimeout);
            break;
        }
    }
}

void OnTxDone( void )
{
    configASSERT(_config != NULL);

    loraMode_t mode = RADIO_MODE_STANDBY;
    if(_config->txCb != NULL) {
       mode = _config->txCb();
    }

    radioEnterMode(mode);
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    configASSERT(_config != NULL);
    loraMode_t mode = RADIO_MODE_STANDBY;

    if(_config->rxCb != NULL) {
        mode = _config->rxCb(payload, size, rssi, snr);
    }

    radioEnterMode(mode);
}

void OnTxTimeout( void )
{
    configASSERT(_config != NULL);

    loraMode_t mode = RADIO_MODE_STANDBY;
    if(_config->txTimeoutCb != NULL) {
       mode = _config->txTimeoutCb();
    }

    radioEnterMode(mode);
}

void OnRxTimeout( void )
{
    configASSERT(_config != NULL);

    loraMode_t mode = RADIO_MODE_STANDBY;
    if(_config->rxTimeoutCb != NULL) {
       mode = _config->rxTimeoutCb();
    }

    radioEnterMode(mode);
}

void OnRxError( void )
{
    configASSERT(_config != NULL);
    loraMode_t mode = RADIO_MODE_STANDBY;
    if(_config->rxErrorCb != NULL) {
       mode = _config->rxErrorCb();
    }

    radioEnterMode(mode);
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

    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );

       return 0;
}

TaskHandle_t pxRadioIrqTaskHandle = NULL;

static void prvRadioIrqTask( void *pvParameters ) {
    (void)pvParameters;

    init_radio();

    Radio.Rx(_rxTimeout);

    pxRadioIrqTaskHandle = xTaskGetCurrentTaskHandle();

    for(;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        Radio.IrqProcess();
    }
}

void loraRadioInit(loraRadioConfig_t *config) {
    configASSERT(config != NULL);

    _config = config;

    BaseType_t xRval = xTaskCreate(
            prvRadioIrqTask,
            "radio",
            512,
            NULL,
            tskIDLE_PRIORITY + 2,
            NULL);

    configASSERT(xRval == pdTRUE);

}
