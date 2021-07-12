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
#include <string.h>
#include "chaac_packet.h"
#include "FreeRTOSLPM.h"
#include "packet.h"
#include "sensor.h"
#include "loraRadio.h"

static weather_packet_v1p1_t packet;

extern __IO uint32_t uwTick;
volatile bool useFreeRTOSTick = false;

const uint32_t *HWID = (const uint32_t *)(UID_BASE);
#define UID (HWID[0] ^ HWID[1] ^ HWID[2])

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


loraMode_t loraRxCallback(uint8_t *buff, size_t len, int16_t rssi, int8_t snr){
    (void)snr;
    (void)rssi;

    // Check packet CRC
    if(packetIsValid(buff, len)) {
        packet_header_t *packetHeader = (packet_header_t *)buff;
        chaac_header_t *chaacPacketHeader = (chaac_header_t *)&packetHeader[1];
        if((chaacPacketHeader->uid == UID) &&
            (chaacPacketHeader->type == PACKET_TYPE_RESET)) {
            NVIC_SystemReset();
        }
    }

    return RADIO_MODE_SLEEP;
}

loraMode_t loraRxTimeoutCallback() {
    return RADIO_MODE_SLEEP;
}

loraMode_t loraRxErrorCallback() {
    return RADIO_MODE_SLEEP;
}

loraMode_t loraTxCallback() {
    loraRadioSetRxTimeout(500);
    return RADIO_MODE_RX;
}

loraMode_t loraTxTimeoutCallback() {
    return RADIO_MODE_SLEEP;
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

void packetTxFn(int16_t len, void* data) {
    loraRadioSend((uint8_t *)data, len);
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

    windRainInit();

    packet.header.type = PACKET_TYPE_WEATHER_V1P1;
    packet.header.uid = UID;
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

        packetTx(sizeof(packet), &packet, packetTxFn);

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

extern SPI_HandleTypeDef hspi1;

void spiSetupFn() {
    MX_SPI1_Init();
}

void spiTeardownFn() {
    HAL_SPI_DeInit(&hspi1);
}

static loraRadioConfig_t loraConfig = {
    .startMode = RADIO_MODE_SLEEP,
    .spiSetupFn = spiSetupFn,
    .spiTeardownFn = spiTeardownFn,
    .rxCb = loraRxCallback,
    .txCb = loraTxCallback,
    .rxTimeoutCb = loraRxTimeoutCallback,
    .txTimeoutCb = loraRxErrorCallback,
    .rxErrorCb = NULL,
};

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

    loraRadioInit(&loraConfig);

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


