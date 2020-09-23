#include <assert.h>
#include <string.h>
#include <console/console.h>
#include <stdio.h>

#include <sysinit/sysinit.h>
#include <os/os.h>
#include <bsp/bsp.h>
#include <hal/hal_gpio.h>
#include <simple_adc/simple_adc.h>
#include <packet/packet.h>
#include <hal/hal_nvreg.h>

#include "chaac_packet.h"
#include "radio/radio.h"
#include "sx126x/sx126x.h"

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


static button_packet_v1p0_t packet;

static RadioEvents_t RadioEvents;

void OnTxDone( void )
{
    Radio.Sleep( );
    hal_gpio_write(LED1_PIN, 0);
    console_printf("TX Done\n");
}

void OnTxTimeout( void )
{
    Radio.Sleep( );  
    console_printf("TX Timeout\n");
    hal_gpio_write(LED1_PIN, 0);
}


int radio_init(void) {
    console_printf("Radio Init\n");

    RadioEvents.TxDone = OnTxDone;
    //RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    //RadioEvents.RxTimeout = OnRxTimeout;
    //RadioEvents.RxError = OnRxError;
    // RadioEvents.CadDone = OnCadDone;
    
    Radio.Init( &RadioEvents );

    Radio.SetChannel( RF_FREQUENCY );

    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

    Radio.Sleep();

    return 0;
}

#define MAIN_TASK_PRI         (99)
#define MAIN_STACK_SIZE       (512)
struct os_task main_task;
os_stack_t main_task_stack[MAIN_STACK_SIZE];


void main_task_fn(void *arg) {

    hal_gpio_init_out(LED1_PIN, 0);
    os_time_delay(os_time_ms_to_ticks32(500));

    radio_init();

    while(1) {
        int32_t rval;
        int32_t result = 0;

        rval = simple_adc_read_ch(BATT_ADC_CH, &result);
        if(rval) {
            console_printf("simple_adc_read_ch error %ld\n", rval);
        } else {
            packet.battery = result * 2;
            console_printf("B: %ld.%ld\n",
                (int32_t)(packet.battery/1000),
                (int32_t)((packet.battery-(int32_t)(packet.battery/1000)*1000)));
        }

        packet.ch1 = hal_gpio_read(CH1_PIN);
        packet.ch2 = hal_gpio_read(CH2_PIN);

        console_printf("Radiotx\n");
        hal_gpio_write(LED1_PIN, 1);
        Radio.Send((uint8_t *)&packet, sizeof(packet));
        
        os_time_delay(OS_TICKS_PER_SEC * 30);
    }
    //simple_adc_uninit();
}

int main(int argc, char **argv) {
    int rc;

    sysinit();

    hal_gpio_init_out(PWR_MODE_PIN, 1);
    hal_gpio_init_in(CH1_PIN, HAL_GPIO_PULL_UP);
    hal_gpio_init_in(CH2_PIN, HAL_GPIO_PULL_UP);

    console_printf("LoRa Button v%d.%d\n", (HW_VERS >> 8), HW_VERS & 0xFF);
    console_printf("UID: %08lX\n", DEVICE_UID);


    simple_adc_init();

    packet.header.type = PACKET_TYPE_BUTTON_V1P0;
    packet.header.uid = DEVICE_UID;

    os_task_init(
        &main_task,
        "main_task",
        main_task_fn,
        NULL,
        MAIN_TASK_PRI,
        OS_WAIT_FOREVER,
        main_task_stack,
        MAIN_STACK_SIZE);

    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    assert(0);
    return rc;
}
