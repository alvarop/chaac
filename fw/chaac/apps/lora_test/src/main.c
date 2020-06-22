#include <assert.h>
#include <string.h>

#include "sysinit/sysinit.h"
#include "os/os.h"
#include "console/console.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"

#include "radio/radio.h"
#include "sx126x/sx126x.h"


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
/*#define LORA_PREAMBLE_LENGTH                        108       // Same for Tx and Rx*/
/*#define LORA_SYMBOL_TIMEOUT                         100       // Symbols*/
/*#define LORA_FIX_LENGTH_PAYLOAD_ON                  false*/
/*#define LORA_IQ_INVERSION_ON                        false*/
/*#define LORA_FIX_LENGTH_PAYLOAD_LEN                 19*/
#define LORA_PREAMBLE_LENGTH                        8       // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0       // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            0
#define BUFFER_SIZE                                 64 // Define the payload size her

typedef enum                       
{  
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT,
    START_CAD,
}States_t;

typedef enum
{
    CAD_FAIL,
    CAD_SUCCESS,
    PENDING,
}CadRx_t;

uint16_t BufferSize = BUFFER_SIZE;   
uint8_t Buffer[BUFFER_SIZE];
   
States_t State = LOWPOWER;
   
int8_t RssiValue = 0;
int8_t SnrValue = 0;

CadRx_t CadRx = CAD_FAIL;
bool RxTimeoutTimerIrqFlag = false;
uint16_t channelActivityDetectedCnt = 0;
uint16_t RxCorrectCnt = 0;
uint16_t RxErrorCnt = 0;
uint16_t RxTimeoutCnt = 0;
uint16_t SymbTimeoutCnt = 0;
int16_t RssiMoy = 0;
int8_t SnrMoy = 0;

//TimerEvent_t CADTimeoutTimer;
#define CAD_TIMER_TIMEOUT       1000        //Define de CAD timer's timeout here
                                 
//TimerEvent_t RxAppTimeoutTimer;
#define RX_TIMER_TIMEOUT        4000        //Define de CAD timer's timeout here

//CAD parameters
#define CAD_SYMBOL_NUM          1 
//#define CAD_SYMBOL_NUM          LORA_CAD_02_SYMBOL
#define CAD_DET_PEAK            22
#define CAD_DET_MIN             10
#define CAD_TIMEOUT_MS          2000
#define NB_TRY                  10

static RadioEvents_t RadioEvents;

int g_led_pin;

void OnTxDone( void )
{
    Radio.Standby( );
    State = TX;
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    
    hal_gpio_write(TX_LED_PIN, 1);
    /*Radio.Standby( );*/
    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
    RssiValue = rssi;
    SnrValue = snr;
    RssiMoy = (((RssiMoy * RxCorrectCnt) + RssiValue) / (RxCorrectCnt + 1));
    SnrMoy = (((SnrMoy * RxCorrectCnt) + SnrValue) / (RxCorrectCnt + 1));
    State = RX;
    payload[size] = 0;
    console_printf("RX! %s %d %d %d\n", payload, size, rssi, snr);
    //console_printf("RX! %d %d %d\n", size, rssi, snr);
    Radio.Rx(RX_TIMEOUT_VALUE);
    hal_gpio_write(TX_LED_PIN, 0);
}

void OnTxTimeout( void )
{
    Radio.Standby( );
    State = TX_TIMEOUT;
}

void OnRxTimeout( void )
{   
    //console_printf("RX Timeout\n");
    /*Radio.Standby( );*/
    Radio.Rx( RX_TIMEOUT_VALUE );   //  Restart Rx
}

void OnRxError( void )
{
    console_printf("RX Err\n");
    Radio.Standby( );
    State = RX_ERROR;
}

void OnCadDone( bool channelActivityDetected)
{
    Radio.Standby( );

    if( channelActivityDetected == true )
    {
        CadRx = CAD_SUCCESS;
    }
    else
    {
        CadRx = CAD_FAIL;
    }
    State = RX;
}

void SX126xConfigureCad( RadioLoRaCadSymbols_t cadSymbolNum, uint8_t cadDetPeak, uint8_t cadDetMin , uint32_t cadTimeout)
{
    SX126xSetDioIrqParams(  IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED, IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED,
                            IRQ_RADIO_NONE, IRQ_RADIO_NONE );
    SX126xSetCadParams( cadSymbolNum, cadDetPeak, cadDetMin, LORA_CAD_ONLY, ((cadTimeout * 1000) / 15.625 ));
}

/*static void CADTimeoutTimeoutIrq( void )*/
/*{*/
    /*Radio.Standby( );*/
    /*State = START_CAD;*/
/*}*/

/*static void RxTimeoutTimerIrq( void )*/
/*{*/
    /*RxTimeoutTimerIrqFlag = true;*/
/*}*/

int init_radio(void) {
    console_printf("Initializing radio\n");

    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;
    RadioEvents.CadDone = OnCadDone;

    //Timer used to restart the CAD
    /*TimerInit( &CADTimeoutTimer, CADTimeoutTimeoutIrq );*/
    /*TimerSetValue( &CADTimeoutTimer, CAD_TIMER_TIMEOUT );*/
    
    //App timmer used to check the RX's end
    /*TimerInit( &RxAppTimeoutTimer, RxTimeoutTimerIrq );*/
    /*TimerSetValue( &RxAppTimeoutTimer, RX_TIMER_TIMEOUT );*/

    console_printf("Radio.Init\n");
    Radio.Init( &RadioEvents );

    // Need TCXO to be on
    SX126xSetDio3AsTcxoCtrl( TCXO_CTRL_2_4V, 5000 );
    console_printf("Enabled TCXO @ 2.4V\n");

    CalibrationParams_t calibParam;
    // Calibrate all blocks
    calibParam.Value = 0x7F;
    console_printf("Calibrating all blocks\n");
    SX126xCalibrate( calibParam );

    console_printf("Radio.SetChannel\n");
    Radio.SetChannel( RF_FREQUENCY );
    
    console_printf("Radio.SetTxConfig\n");\
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );

    /*SX126xConfigureCad( CAD_SYMBOL_NUM,*/
                        /*CAD_DET_PEAK,CAD_DET_MIN,*/
                        /*CAD_TIMEOUT_MS);            // Configure the CAD*/
                        /*Radio.StartCad( );          // do the config and lunch first */

    return 0;
}


#define RADIO_TASK_PRI         (90)
#define RADIO_STACK_SIZE       (1024)
struct os_task radio_task;
os_stack_t radio_task_stack[RADIO_STACK_SIZE];


#define BLINK_TASK_PRI         (99)
#define BLINK_STACK_SIZE       (64)
struct os_task blink_task;
os_stack_t blink_task_stack[BLINK_STACK_SIZE];



void blink_task_fn(void *arg) {

    hal_gpio_init_out(STATUS_LED_PIN, 0);

    while(1) {
        os_time_delay(OS_TICKS_PER_SEC/4);
        hal_gpio_toggle(STATUS_LED_PIN);
    }

}

void radio_task_fn(void *arg) {

    hal_gpio_init_out(E22_TXEN, 0);
    hal_gpio_init_out(E22_RXEN, 1);
    hal_gpio_init_out(TX_LED_PIN, 0);
    hal_gpio_init_out(RX_LED_PIN, 0);
 
    init_radio();

    // Start receiving
    Radio.Rx(RX_TIMEOUT_VALUE);
    while (1) {
        os_time_delay(1);

        /*hal_gpio_toggle(g_led_pin);*/

        /*console_printf("Blink...\n");*/
        Radio.IrqProcess( );
    }

}


int
main(int argc, char **argv)
{
    sysinit();

    hal_gpio_init_out(E22_TXEN, 0);
    hal_gpio_init_out(E22_RXEN, 1);


    os_task_init(
        &blink_task,
        "blink_task",
        blink_task_fn,
        NULL,
        BLINK_TASK_PRI,
        OS_WAIT_FOREVER,
        blink_task_stack,
        BLINK_STACK_SIZE);

    os_task_init(
        &radio_task,
        "radio_task",
        radio_task_fn,
        NULL,
        RADIO_TASK_PRI,
        OS_WAIT_FOREVER,
        radio_task_stack,
        RADIO_STACK_SIZE);


    while(1) {
        os_eventq_run(os_eventq_dflt_get());
    }

    assert(0);

    return 0;
}

