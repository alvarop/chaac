#include "stm32l4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
// #include "printf.h"
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
#include "chaac_packet.h"
#include "FreeRTOSLPM.h"

static weather_packet_v1p0_t packet;


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

static RadioEvents_t RadioEvents;

extern __IO uint32_t uwTick;
volatile bool useFreeRTOSTick = false;

uint32_t HAL_GetTick(void) {
    if(useFreeRTOSTick) {
        return xTaskGetTickCount();
    } else {
        return uwTick;
    }
}

void HAL_Delay(uint32_t delay) {
    if(useFreeRTOSTick) {
        vTaskDelay(delay);
    } else {
        uint32_t tickstart = HAL_GetTick();
        uint32_t wait = delay;

        /* Add a period to guaranty minimum wait */
        if (wait < HAL_MAX_DELAY) {
            wait += (uint32_t)uwTickFreq;
        }

        while ((HAL_GetTick() - tickstart) < wait) {};
    }
}

void OnTxDone( void )
{
    // printf("txdone\n");
    Radio.Sleep( );
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
}

int init_radio(void) {
    // printf("Initializing radio\n");

    MX_SPI1_Init();

    RadioEvents.TxDone = OnTxDone;
    // RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    // RadioEvents.RxTimeout = OnRxTimeout;
    // RadioEvents.RxError = OnRxError;
    // RadioEvents.CadDone = OnCadDone;

    // printf("Radio.Init\n");
    SX126xIoInit();

    Radio.Init( &RadioEvents );

    // printf("Radio.SetTxConfig\n");
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

    // Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
    //                                LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
    //                                LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
    //                                0, true, 0, 0, LORA_IQ_INVERSION_ON, true );

    Radio.Sleep( );

    HAL_SPI_DeInit(&hspi1);

    return 0;
}

static uint32_t prvAdcGetSampleMv(uint32_t ulChannel) {
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
    return ((uint32_t)lResult);
}

const uint32_t *HWID = (const uint32_t *)(UID_BASE);

static void showError(uint32_t error) {
    for(uint32_t x = 0; x < 20; x++) {
        LL_GPIO_SetOutputPin(LED1_GPIO_Port, LED1_Pin);
        vTaskDelay(25);
        LL_GPIO_ResetOutputPin(LED1_GPIO_Port, LED1_Pin);
        vTaskDelay(25);
    }

    vTaskDelay(475);

    for(uint32_t x = 0; x < error; x++) {
        LL_GPIO_SetOutputPin(LED1_GPIO_Port, LED1_Pin);
        vTaskDelay(250);
        LL_GPIO_ResetOutputPin(LED1_GPIO_Port, LED1_Pin);
        vTaskDelay(250);
    }

    vTaskDelay(250);

    for(uint32_t x = 0; x < 20; x++) {
        LL_GPIO_SetOutputPin(LED1_GPIO_Port, LED1_Pin);
        vTaskDelay(25);
        LL_GPIO_ResetOutputPin(LED1_GPIO_Port, LED1_Pin);
        vTaskDelay(25);
    }
}

static void prvMainTask( void *pvParameters ) {
    (void)pvParameters;

    LL_GPIO_ResetOutputPin(RADIO_NRST_GPIO_Port, RADIO_NRST_Pin);

    xIOI2cInit(&hi2c1);
    vWindRainInit();

    init_radio();

    packet.header.type = PACKET_TYPE_WEATHER_V1P0;
    packet.header.uid = HWID[0] ^ HWID[1] ^ HWID[2];
    packet.sample = 0;

    uint32_t ulRval = ulSht3xInit(&hi2c1, SHT3x_ADDR);
    if(ulRval) {
        showError(1);
    }

    ulRval = dps368_init(&hi2c1);
    if(ulRval) {
        showError(2);
    }

    xIOI2cDeInit(&hi2c1);

    for(;;) {
        // Enable sensor power rail
        LL_GPIO_ResetOutputPin(SNS_3V3_EN_GPIO_Port, SNS_3V3_EN_Pin);

        xIOI2cInit(&hi2c1);
        xIOAdcInit(&hadc1);

        int16_t sTemperature, sHumidity;
        ulRval = ulSht3xRead(&hi2c1, SHT3x_ADDR, &sTemperature, &sHumidity);
        if(ulRval == 0) {
            packet.temperature = sTemperature;
            packet.humidity = sHumidity;
        } else {
            packet.temperature = -27300;
            packet.humidity = 0;
        }

        float fTemperature, fPressure;
        ulRval = dps368_measure_temp_once(&fTemperature);
        if (ulRval != 0) {
            // printf("Error reading DPS368 Temperature (%d)\n", ulRval);
        }
        ulRval = dps368_measure_pressure_once(&fPressure);
        if (ulRval != 0) {
            // printf("Error reading DPS368 Pressure (%d)\n", ulRval);
        }

        if (ulRval == 0) {
            packet.pressure = (int16_t)(((fPressure - 100000.0)));
            // printf("T: %0.2f C T: %0.2f hPa\n", fTemperature, fPressure/100.0);
        } else {
            // set unrealistic pressure on error
            packet.pressure = INT16_MIN;
        }

        packet.battery = prvAdcGetSampleMv(ADC_CHANNEL_7) * 2;
        packet.solar_panel = prvAdcGetSampleMv(ADC_CHANNEL_6) * 2;

        packet.rain = ulWindRainGetRain()/2794;

        // Store wind speed in kph * 100
        packet.wind_speed = ulWindRainGetSpeed()/10;
        packet.wind_dir = xWindRainGetDir(prvAdcGetSampleMv(ADC_CHANNEL_5));

        vWindRainClearRain();

        MX_SPI1_Init();
        Radio.Send((uint8_t *)&packet, sizeof(packet));
        HAL_SPI_DeInit(&hspi1);

        packet.sample++;

        xIOI2cDeInit(&hi2c1);
        xIOAdcDeInit(&hadc1);
        // Disable sensor power rail
        LL_GPIO_SetOutputPin(SNS_3V3_EN_GPIO_Port, SNS_3V3_EN_Pin);
        vTaskDelay(29925);
    }
}

TaskHandle_t pxRadioIrqTaskHandle = NULL;
extern SPI_HandleTypeDef hspi1;

static void prvRadioIrqTask( void *pvParameters ) {
    (void)pvParameters;

    pxRadioIrqTaskHandle = xTaskGetCurrentTaskHandle();

    for(;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        MX_SPI1_Init();
        Radio.IrqProcess();
        HAL_SPI_DeInit(&hspi1);
    }
}

int main(void) {
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();

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

    lpmInit();
    lpmEnable();

    useFreeRTOSTick = true;
    HAL_SuspendTick();

    vTaskStartScheduler();

    __builtin_unreachable();
}


