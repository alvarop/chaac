#include "stm32l4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "printf.h"
#include "IOI2c.h"
#include "IOAdc.h"
#include "debug.h"
#include "sht3x.h"
#include "dps368.h"
#include "windrain.h"
#include "adc.h"
#include "i2c.h"
#include "spi.h"
#include "gpio.h"
#include "radio.h"
#include "sx126x.h"
#include "sx126x-board.h"
#include <string.h>


extern I2C_HandleTypeDef hi2c1;

static uint32_t ulRainTicks = 0;
static uint32_t ulWindSpeedTicks = 0;

void vWRRainIrq() {
    ulRainTicks++;
}

void vWRWindSpeedIrq() {
    ulWindSpeedTicks++;
}

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

void OnTxDone( void )
{
    printf("txdone\n");
    Radio.Standby( );
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    const uint16_t total_size = size;// + sizeof(chaac_lora_rxinfo_t);
    if(total_size <= BUFFER_SIZE) {
        memcpy(txbuff, payload, size);
        (void)rssi;
        (void)snr;

        // chaac_lora_rxinfo_t *footer = (chaac_lora_rxinfo_t *)&txbuff[size];
        // footer->rssi = rssi;
        // footer->snr = snr;

        // packet_tx(size + sizeof(chaac_lora_rxinfo_t), txbuff);
        /*for(uint16_t byte = 0; byte < size; byte++) {
            printf("%02X ", payload[byte]);
        }
        printf("RSSI:%d SNR:%d\n", rssi, snr);
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
    printf("RX Timeout\n");
    /*Radio.Standby( );*/
    Radio.Rx( RX_TIMEOUT_VALUE );   //  Restart Rx
}

void OnRxError( void )
{
    printf("RX Err\n");
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

    printf("Radio.Init\n");
    SX126xIoInit();

    Radio.Init( &RadioEvents );

    printf("Radio.SetTxConfig\n");
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

    // Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
    //                                LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
    //                                LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
    //                                0, true, 0, 0, LORA_IQ_INVERSION_ON, true );

       return 0;
}

static float prvAdcGetSample(uint32_t ulChannel) {
    int32_t lResult = 0;

    ADC_ChannelConfTypeDef xConfig = {0};

    xConfig.Rank = ADC_REGULAR_RANK_1;
    xConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
    xConfig.SingleDiff = ADC_SINGLE_ENDED;
    xConfig.OffsetNumber = ADC_OFFSET_NONE;
    xConfig.Offset = 0;

    xConfig.Channel = ulChannel;
    xIOAdcChannelConfig(&hadc1, &xConfig);

    xIOAdcReadMv(&hadc1, &lResult);
    return ((float)lResult / 1000.0);
}

static char packet[] = "hello\n";
static void prvMainTask( void *pvParameters ) {
    (void)pvParameters;

    LL_GPIO_ResetOutputPin(RADIO_NRST_GPIO_Port, RADIO_NRST_Pin);

#ifdef BUILD_DEBUG
    vDebugInit();
    printf("Chaac FW\n");
#endif

    xIOI2cInit(&hi2c1);
    xIOAdcInit(&hadc1);
    vWindRainInit();

    init_radio();

    uint32_t ulRval = ulSht3xInit(&hi2c1, SHT3x_ADDR);
    if(ulRval == 0) {
        printf("SHT3x Initialized Successfully!\n");
    } else {
        printf("Error initializing SHT3x (%ld)\n", ulRval);
    }

    ulRval = dps368_init(&hi2c1);
    if(ulRval == 0) {
        printf("DPS368 Initialized Successfully!\n");
    } else {
        printf("Error initializing DPS368 (%ld)\n", ulRval);
    }

    // Enable sensor power rail
    LL_GPIO_ResetOutputPin(SNS_3V3_EN_GPIO_Port, SNS_3V3_EN_Pin);

    for(;;) {
        LL_GPIO_SetOutputPin(LED1_GPIO_Port, LED1_Pin);
        vTaskDelay(25);
        LL_GPIO_ResetOutputPin(LED1_GPIO_Port, LED1_Pin);

        int16_t sTemperature, sHumidity;
        ulRval = ulSht3xRead(&hi2c1, SHT3x_ADDR, &sTemperature, &sHumidity);
        if(ulRval == 0) {
            printf("T: %0.2fC H: %0.2f %RH\n", (float)sTemperature/100.0, (float)sHumidity/100.0);
        } else {
            printf("Error reading from SHT3x (%ld)\n", ulRval);
        }

        float fTemperature, fPressure;

        ulRval = dps368_measure_temp_once(&fTemperature);
        if (ulRval != 0) {
            printf("Error reading DPS368 Temperature (%d)\n", ulRval);
        }
        ulRval = dps368_measure_pressure_once(&fPressure);
        if (ulRval != 0) {
            printf("Error reading DPS368 Pressure (%d)\n", ulRval);
        }

        if (ulRval == 0) {
            printf("T: %0.2f C T: %0.2f hPa\n", fTemperature, fPressure/100.0);
        }

        printf("VSOLAR: %0.3f V\n", prvAdcGetSample(ADC_CHANNEL_6) * 2.0);
        printf("BATT: %0.3f V\n", prvAdcGetSample(ADC_CHANNEL_7) * 2.0);

        printf("Wind: %0.2f kph @ %0.1f\n", (float)ulWindRainGetSpeed()/1000.0, (float)sWindRainGetDirDegrees()/10.0);
        printf("Rain: %0.3f mm\n", (float)ulWindRainGetRain()/10000.0);
        vWindRainClearRain();

        printf("Tx packet\n");
        Radio.Send((uint8_t *)&packet, sizeof(packet));
        vTaskDelay(9925);
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

int main(void) {
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_SPI1_Init();

    BaseType_t xRval = xTaskCreate(
            prvMainTask,
            "main",
            512,
            NULL,
            tskIDLE_PRIORITY + 1,
            NULL);

    configASSERT(xRval == pdTRUE);

    xRval = xTaskCreate(
            prvRadioIrqTask,
            "radio",
            512,
            NULL,
            tskIDLE_PRIORITY + 2,
            NULL);

    configASSERT(xRval == pdTRUE);

    vTaskStartScheduler();

    __builtin_unreachable();
}


