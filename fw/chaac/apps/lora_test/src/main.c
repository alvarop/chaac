#include <assert.h>
#include <string.h>

#include "sysinit/sysinit.h"
#include "os/os.h"
#include "console/console.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"

#include <packet/packet.h>
#include <raw_uart/raw_uart.h>

#include "radio/radio.h"
#include "sx126x/sx126x.h"
#include "chaac_packet.h"

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
#define BUFFER_SIZE                                 64 // Define the payload size her

uint8_t txbuff[BUFFER_SIZE];
   
static RadioEvents_t RadioEvents;

void packet_tx_fn(int16_t len, void* data) {

#if HW_VERSION == 0
    hal_gpio_init_out(RX_LED_PIN, 1);
#endif
    
    raw_uart_tx(len, data);

#if HW_VERSION == 0
    hal_gpio_init_out(RX_LED_PIN, 0);
#endif
}

void OnTxDone( void )
{
    Radio.Standby( );
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    const uint16_t total_size = size + sizeof(chaac_lora_rxinfo_t);
    if(total_size <= BUFFER_SIZE) {
        memcpy(txbuff, payload, size);
    
        chaac_lora_rxinfo_t *footer = (chaac_lora_rxinfo_t *)&txbuff[size];
        footer->rssi = rssi;
        footer->snr = snr;

        packet_tx(size + sizeof(chaac_lora_rxinfo_t), txbuff);
        /*for(uint16_t byte = 0; byte < size; byte++) {
            console_printf("%02X ", payload[byte]);
        }
        console_printf("RSSI:%d SNR:%d\n", rssi, snr);
        */
        Radio.Rx(RX_TIMEOUT_VALUE);
    }
}

void OnTxTimeout( void )
{
    Radio.Standby( );
}

void OnRxTimeout( void )
{   
    //console_printf("RX Timeout\n");
    /*Radio.Standby( );*/
    Radio.Rx( RX_TIMEOUT_VALUE );   //  Restart Rx
}

void OnRxError( void )
{
    /*console_printf("RX Err\n");*/
    /*Radio.Standby( );*/
    Radio.Rx( RX_TIMEOUT_VALUE );   //  Restart Rx
}

int init_radio(void) {
    /*console_printf("Initializing radio\n");*/

#if HW_VERSION == 0
    hal_gpio_init_out(TX_LED_PIN, 0);
    hal_gpio_init_out(RX_LED_PIN, 0);
#endif

    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;
//    RadioEvents.CadDone = OnCadDone;

    /*console_printf("Radio.Init\n");*/
    Radio.Init( &RadioEvents );

    /*console_printf("Radio.SetTxConfig\n");\*/
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

#define BLINK_TASK_PRI         (99)
#define BLINK_STACK_SIZE       (64)
struct os_task blink_task;
os_stack_t blink_task_stack[BLINK_STACK_SIZE];



void blink_task_fn(void *arg) {

    hal_gpio_init_out(STATUS_LED_PIN, 0);

    while(1) {
        os_time_delay(OS_TICKS_PER_SEC);
        hal_gpio_toggle(STATUS_LED_PIN);
    }

}

int
main(int argc, char **argv)
{
    sysinit();

    raw_uart_init(NULL);
    packet_init_tx_fn(packet_tx_fn);

    os_task_init(
        &blink_task,
        "blink_task",
        blink_task_fn,
        NULL,
        BLINK_TASK_PRI,
        OS_WAIT_FOREVER,
        blink_task_stack,
        BLINK_STACK_SIZE);

    init_radio();

    Radio.Rx(RX_TIMEOUT_VALUE);

    while(1) {
        os_eventq_run(os_eventq_dflt_get());
    }

    assert(0);

    return 0;
}

