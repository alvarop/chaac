#include <assert.h>
#include <string.h>

#include "sysinit/sysinit.h"
#include "os/os.h"
#include "console/console.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"

#include <packet/packet.h>

#include "radio/radio.h"
#include "sx126x/sx126x.h"
#include "chaac_packet.h"

#define RF_FREQUENCY 915000000

#define TX_OUTPUT_POWER 0 

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

static range_test_packet_v1p0_t packet;
static RadioEvents_t RadioEvents;

static uint32_t sample_num;


void OnTxDone( void )
{
    Radio.Sleep( );
    hal_gpio_write(MYNEWT_VAL(SX126X_N_RXTX_PIN), 0);
    hal_gpio_write(LED1_PIN, 0);
    console_printf("TX Done\n");
}

void OnTxTimeout( void )
{
    Radio.Sleep( );  
    console_printf("TX Timeout\n");
    hal_gpio_write(MYNEWT_VAL(SX126X_N_RXTX_PIN), 0);
    hal_gpio_write(LED1_PIN, 0);
}

void radio_tx_config(uint8_t pwr) {
    Radio.SetTxConfig( MODEM_LORA, pwr, 0, LORA_BANDWIDTH,
                       LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                       LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                       true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

}

int radio_init(void) {

    
    console_printf("Radio Init\n");
    hal_gpio_init_out(MYNEWT_VAL(SX126X_N_RXTX_PIN), 0);
    hal_gpio_init_out(MYNEWT_VAL(SX126X_RXTX_PIN), 0);

    RadioEvents.TxDone = OnTxDone;
    //RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    //RadioEvents.RxTimeout = OnRxTimeout;
    //RadioEvents.RxError = OnRxError;
    // RadioEvents.CadDone = OnCadDone;
   
    Radio.Init( &RadioEvents );


    // Need TCXO to be on
    SX126xSetDio3AsTcxoCtrl( TCXO_CTRL_2_4V, 5000 );

    CalibrationParams_t calibParam;
    // Calibrate all blocks
    calibParam.Value = 0x7F;
    SX126xCalibrate( calibParam );

    Radio.SetChannel( RF_FREQUENCY );

    radio_tx_config(-3);

    /*Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );*/

    /*Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );*/

    Radio.Sleep();

    return 0;
}

  
#define BLINK_TASK_PRI         (99)
#define BLINK_STACK_SIZE       (64)
struct os_task blink_task;
os_stack_t blink_task_stack[BLINK_STACK_SIZE];


#define MIN_TX_POWER (-3)
#define MAX_TX_POWER (22)

void blink_task_fn(void *arg) {
    
    hal_gpio_init_out(LED1_PIN, 0);

    packet.header.type = PACKET_TYPE_RANGE_V1P0; 
    packet.header.uid = DEVICE_UID;

    radio_init();
    int8_t tx_pwr = -3;

    while(1) {
        os_time_delay(OS_TICKS_PER_SEC * 2);
        radio_tx_config(tx_pwr);
        packet.sample = sample_num++;
        packet.tx_pwr = tx_pwr;

        hal_gpio_write(MYNEWT_VAL(SX126X_N_RXTX_PIN), 1);
        hal_gpio_write(LED1_PIN, 1);
        Radio.Send((uint8_t *)&packet, sizeof(packet));

        if (++tx_pwr > MAX_TX_POWER) {
            tx_pwr = MIN_TX_POWER;
        }
    }

}

int
main(int argc, char **argv)
{
    sysinit();

    os_task_init(
        &blink_task,
        "blink_task",
        blink_task_fn,
        NULL,
        BLINK_TASK_PRI,
        OS_WAIT_FOREVER,
        blink_task_stack,
        BLINK_STACK_SIZE);


    while(1) {
        os_eventq_run(os_eventq_dflt_get());
    }

    assert(0);

    return 0;
}

