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
#include "packet.h"
#include "sensor.h"

static weather_packet_v1p1_t packet;


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

static uint8_t txbuff[BUFFER_SIZE];
void loraRxCallback(uint8_t *buff, size_t len, int16_t rssi, int8_t snr){

    // Check packet CRC
    if(packetIsValid(buff, len) && (len <= BUFFER_SIZE)) {
        packet_header_t *header = (packet_header_t *)buff;

        memcpy(txbuff, (void *)&header[1], header->len);

        chaac_lora_rxinfo_t *footer = (chaac_lora_rxinfo_t *)&txbuff[len];
        footer->rssi = rssi;
        footer->snr = snr;

        packetTx(len + sizeof(chaac_lora_rxinfo_t), txbuff);
    }

    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    vTaskDelay(50);
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
}

void packetTxFn(int16_t len, void* data) {
    MX_SPI1_Init();
    Radio.Send((uint8_t *)data, len);
    HAL_SPI_DeInit(&hspi1);
}

static sensor_t shtTemperature;
static sensor_t shtHumidity;
static sensor_t dpsPressure;
static sensor_t dpsTemperature;
static sensor_t vBatt;
static sensor_t vSolar;

static void mainTask( void *pvParameters ) {
    (void)pvParameters;

    LL_GPIO_ResetOutputPin(RADIO_NRST_GPIO_Port, RADIO_NRST_Pin);

    packetInitTxFn(packetTxFn);
    windRainInit();

    init_radio();

    packet.header.type = PACKET_TYPE_WEATHER_V1P1;
    packet.header.uid = HWID[0] ^ HWID[1] ^ HWID[2];
    packet.sample = 0;

    for(;;) {
        vTaskDelay(60 * 1000);

        float fVal;

        if(sensorGetAvg(&shtTemperature, &fVal)) {
            packet.temperature = (int16_t)fVal;
            sensorClearSamples(&shtTemperature);
        } else {
            packet.temperature = -27300;
        }

        if(sensorGetAvg(&shtHumidity, &fVal)) {
            packet.humidity = (uint16_t)fVal;
            sensorClearSamples(&shtHumidity);
        } else {
            packet.humidity = 0;
        }

        if(sensorGetAvg(&dpsPressure, &fVal)) {
            packet.pressure = (uint16_t)fVal;
            sensorClearSamples(&dpsPressure);
        } else {
            packet.pressure = INT16_MIN;
        }

        if(sensorGetAvg(&dpsTemperature, &fVal)) {
            packet.alt_temperature = (int16_t)(fVal * 100);
            sensorClearSamples(&dpsTemperature);
        } else {
            packet.alt_temperature = -27300;
        }

        if(sensorGetAvg(&vBatt, &fVal)) {
            packet.battery = (uint16_t)fVal;
            sensorClearSamples(&vBatt);
        } else {
            packet.battery = 0;
        }

        if(sensorGetAvg(&vSolar, &fVal)) {
            packet.solar_panel = (uint16_t)fVal;
            sensorClearSamples(&vSolar);
        } else {
            packet.solar_panel = 0;
        }

        packet.rain = windRainGetRain()/2794;

        // Store wind speed in kph * 100
        packet.wind_speed = windRainGetSpeed()/10;
        packet.gust_speed = windRainGetGust()/10;

        // Enable sensor power rail
        LL_GPIO_ResetOutputPin(SNS_3V3_EN_GPIO_Port, SNS_3V3_EN_Pin);

        // Wait for power to make it out and back for a good measurement
        vTaskDelay(25);

        xIOAdcInit(&hadc1);
        packet.wind_dir_deg = windRainGetDirDegrees(prvAdcGetSampleMv(ADC_CHANNEL_5));
        xIOAdcDeInit(&hadc1);

        // Disable sensor power rail
        LL_GPIO_SetOutputPin(SNS_3V3_EN_GPIO_Port, SNS_3V3_EN_Pin);

        windRainClearRain();

        packetTx(sizeof(packet), &packet);

        packet.sample++;
    }
}

static void sensorsTask( void *pvParameters ) {
    (void)pvParameters;

    LL_GPIO_ResetOutputPin(RADIO_NRST_GPIO_Port, RADIO_NRST_Pin);

    xIOI2cInit(&hi2c1);

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

        xIOI2cInit(&hi2c1);

        int16_t sTemperature, sHumidity;
        ulRval = ulSht3xRead(&hi2c1, SHT3x_ADDR, &sTemperature, &sHumidity);
        if(ulRval == 0) {
            sensorAddSample(&shtTemperature, sTemperature);
            sensorAddSample(&shtHumidity, sHumidity);
        }

        float fTemperature, fPressure;
        ulRval = dps368_measure_temp_once(&fTemperature);
        if (ulRval == 0) {
            sensorAddSample(&dpsTemperature, fTemperature);
        } else {
            // printf("Error reading DPS368 Temperature (%d)\n", ulRval);
        }
        ulRval = dps368_measure_pressure_once(&fPressure);
        if (ulRval == 0) {
            sensorAddSample(&dpsPressure, (fPressure - 100000.0));
        } else  {
            // printf("Error reading DPS368 Pressure (%d)\n", ulRval);
        }
        xIOI2cDeInit(&hi2c1);

        xIOAdcInit(&hadc1);
        sensorAddSample(&vBatt, prvAdcGetSampleMv(ADC_CHANNEL_7) * 2);
        sensorAddSample(&vSolar, prvAdcGetSampleMv(ADC_CHANNEL_6) * 2);
        xIOAdcDeInit(&hadc1);

        vTaskDelay(1000);
    }
}

TaskHandle_t pxRadioIrqTaskHandle = NULL;
extern SPI_HandleTypeDef hspi1;

static void radioIrqTask( void *pvParameters ) {
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
            mainTask,
            "main",
            512,
            NULL,
            3,
            NULL);

    configASSERT(xRval == pdTRUE);

    xRval = xTaskCreate(
            radioIrqTask,
            "radio",
            512,
            NULL,
            4,
            NULL);

    configASSERT(xRval == pdTRUE);

    xRval = xTaskCreate(
        sensorsTask,
        "sensors",
        512,
        NULL,
        1,
        NULL);

    configASSERT(xRval == pdTRUE);

    lpmInit();
    lpmEnable();

    useFreeRTOSTick = true;
    HAL_SuspendTick();

    vTaskStartScheduler();

    __builtin_unreachable();
}


