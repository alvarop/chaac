#include "stm32l4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "printf.h"
#include "debug.h"
#include "gpio.h"
#include "usb_device.h"
// #include "spi.h"
// #include "usart.h"
// #include "radio.h"
// #include "sx126x.h"
// #include <string.h>
// #include "sx126x-board.h"
// #include "chaac_packet.h"
// #include "packet.h"

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

// uint8_t txbuff[BUFFER_SIZE];

#if 0
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
    // printf("rx\n");
}

void OnTxTimeout( void )
{
    Radio.Standby( );
}

void OnRxTimeout( void )
{
    //printf("RX Timeout\n");
    /*Radio.Standby( );*/
    Radio.Rx( RX_TIMEOUT_VALUE );   //  Restart Rx
}

void OnRxError( void )
{
    /*printf("RX Err\n");*/
    /*Radio.Standby( );*/
    Radio.Rx( RX_TIMEOUT_VALUE );   //  Restart Rx
}

int init_radio(void) {
    /*printf("Initializing radio\n");*/

    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;
//    RadioEvents.CadDone = OnCadDone;

    SX126xIoInit();

    Radio.Init( &RadioEvents );

    // printf("Radio.SetTxConfig\n");
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

void prvPacketTxFn(int16_t len, void* data) {


    uint8_t *pucByte = (uint8_t *)data;
    while(len > 0){
      // Wait until ready to transmit
      while(!LL_USART_IsActiveFlag_TXE(USART2)){};

      LL_USART_TransmitData8(USART2, *pucByte++);

      len--;
    }

}



TaskHandle_t pxRadioIrqTaskHandle = NULL;

static void prvRadioIrqTask( void *pvParameters ) {
    (void)pvParameters;

    pxRadioIrqTaskHandle = xTaskGetCurrentTaskHandle();

    for(;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        Radio.IrqProcess();
    }
}
#endif


static void prvMainTask( void *pvParameters ) {
    (void)pvParameters;

#ifdef BUILD_DEBUG
    // vDebugInit();
    // printf("Chaac FW\n");
#endif

    // init_radio();

    // vPacketInitTxFn(prvPacketTxFn);

    // Radio.Rx(RX_TIMEOUT_VALUE);

    for(;;) {
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
        vTaskDelay(50);
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
        vTaskDelay(400);

        HAL_GPIO_WritePin(RADIO_TXEN_GPIO_Port, RADIO_TXEN_Pin, GPIO_PIN_SET);
        vTaskDelay(50);
        HAL_GPIO_WritePin(RADIO_TXEN_GPIO_Port, RADIO_TXEN_Pin, GPIO_PIN_RESET);
        vTaskDelay(400);
    }
}

int main(void) {
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_USB_DEVICE_Init();
    // MX_SPI1_Init();
    // MX_USART2_UART_Init();

    BaseType_t xRval = xTaskCreate(
            prvMainTask,
            "main",
            512,
            NULL,
            tskIDLE_PRIORITY + 1,
            NULL);

    configASSERT(xRval == pdTRUE);

#if 0
    xRval = xTaskCreate(
            prvRadioIrqTask,
            "radio",
            512,
            NULL,
            tskIDLE_PRIORITY + 2,
            NULL);

    configASSERT(xRval == pdTRUE);
#endif

    vTaskStartScheduler();

    __builtin_unreachable();
}


