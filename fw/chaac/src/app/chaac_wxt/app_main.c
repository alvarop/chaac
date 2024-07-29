#include "FreeRTOS.h"
#include "main.h"
#include "stm32l4xx_hal.h"
#include "task.h"
// #include "printf.h"
#include <string.h>

#include "FreeRTOSLPM.h"
#include "IOAdc.h"
#include "adc.h"
#include "chaac_packet.h"
#include "debug.h"
#include "gpio.h"
#include "info.h"
#include "iwdg.h"
#include "loraRadio.h"
#include "memfault/components.h"
#include "memfault/core/data_packetizer.h"
#include "packet.h"
#include "radio.h"
#include "sensor.h"
#include "serial.h"
#include "spi.h"
#include "usart.h"
#include "vaisala.h"

static weather_packet_v1p1_t packet;

extern __IO uint32_t uwTick;
volatile bool useFreeRTOSTick = false;

uint32_t HAL_GetTick(void) {
  if (useFreeRTOSTick) {
    return xTaskGetTickCount();
  } else {
    return uwTick;
  }
}

void HAL_Delay(uint32_t delay) {
  if (useFreeRTOSTick) {
    vTaskDelay(delay);
  } else {
    uint32_t tickstart = HAL_GetTick();
    uint32_t wait = delay;

    /* Add a period to guaranty minimum wait */
    if (wait < HAL_MAX_DELAY) {
      wait += (uint32_t)uwTickFreq;
    }

    while ((HAL_GetTick() - tickstart) < wait) {
    };
  }
}

loraMode_t loraRxCallback(uint8_t *buff, size_t len, int16_t rssi, int8_t snr) {
  (void)snr;
  (void)rssi;

  // Check packet CRC
  if (packetIsValid(buff, len)) {
    packet_header_t *packetHeader = (packet_header_t *)buff;
    chaac_header_t *chaacPacketHeader = (chaac_header_t *)&packetHeader[1];
    if ((chaacPacketHeader->uid == getHWID()) &&
        (chaacPacketHeader->type == PACKET_TYPE_RESET)) {
      NVIC_SystemReset();
    }
  }

  return RADIO_MODE_SLEEP;
}

loraMode_t loraRxTimeoutCallback() { return RADIO_MODE_SLEEP; }

loraMode_t loraRxErrorCallback() { return RADIO_MODE_SLEEP; }

loraMode_t loraTxCallback() {
  loraRadioSetRxTimeout(500);
  return RADIO_MODE_RX;
}

loraMode_t loraTxTimeoutCallback() { return RADIO_MODE_SLEEP; }

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
  for (uint32_t x = 0; x < 20; x++) {
    LL_GPIO_SetOutputPin(LED1_GPIO_Port, LED1_Pin);
    vTaskDelay(25);
    LL_GPIO_ResetOutputPin(LED1_GPIO_Port, LED1_Pin);
    vTaskDelay(25);
  }

  vTaskDelay(475);

  for (uint32_t x = 0; x < error; x++) {
    LL_GPIO_SetOutputPin(LED1_GPIO_Port, LED1_Pin);
    vTaskDelay(250);
    LL_GPIO_ResetOutputPin(LED1_GPIO_Port, LED1_Pin);
    vTaskDelay(250);
  }

  vTaskDelay(250);

  for (uint32_t x = 0; x < 20; x++) {
    LL_GPIO_SetOutputPin(LED1_GPIO_Port, LED1_Pin);
    vTaskDelay(25);
    LL_GPIO_ResetOutputPin(LED1_GPIO_Port, LED1_Pin);
    vTaskDelay(25);
  }
}

void packetTxFn(uint16_t len, void *data) {
  loraRadioSend((uint8_t *)data, len);
}

// int32_t winGetDirMv() {
//     int32_t mv = 0;

//     // Enable sensor power rail
//     LL_GPIO_ResetOutputPin(SNS_3V3_EN_GPIO_Port, SNS_3V3_EN_Pin);

//     // Wait for power to make it out and back for a good measurement
//     vTaskDelay(25);

//     xIOAdcInit(&hadc1);
//     mv = prvAdcGetSampleMv(ADC_CHANNEL_5);
//     xIOAdcDeInit(&hadc1);

//     // Disable sensor power rail
//     LL_GPIO_SetOutputPin(SNS_3V3_EN_GPIO_Port, SNS_3V3_EN_Pin);

//     return mv;
// }

// static windDirCfg_t windCfg = {&winGetDirMv};

// static sensor_t shtTemperature;
// static sensor_t shtHumidity;
// static sensor_t dpsPressure;
// static sensor_t dpsTemperature;
static sensor_t vBatt;
static sensor_t vSolar;

#define MEMFAULT_DATA_MAX_LEN 256
static void sendMemfaultData() {
  uint8_t *buf = pvPortMalloc(MEMFAULT_DATA_MAX_LEN);
  memfault_packet_t *packet = (memfault_packet_t *)buf;
  packet->header.uid = getHWID();
  packet->header.type = PACKET_TYPE_MEMFAULT;
  size_t buf_len = MEMFAULT_DATA_MAX_LEN - sizeof(memfault_packet_t);
  if (memfault_packetizer_get_chunk(&packet[1], &buf_len)) {
    packet->len = buf_len;
    packetTx(buf_len + sizeof(memfault_packet_t), buf, packetTxFn);
  }
  vPortFree(buf);
}

// Wait up to this amount before sending a message
// (in case there's not vaisala data to send)
static uint32_t max_message_interval_ms = 65 * 1000;
static void mainTask(void *pvParameters) {
  (void)pvParameters;

  LL_GPIO_ResetOutputPin(RADIO_NRST_GPIO_Port, RADIO_NRST_Pin);

  // windRainInit(&windCfg);

  while (!isRadioReady()) {
    vTaskDelay(100);
  }

  sendMemfaultData();

  packet.header.type = PACKET_TYPE_WEATHER_V1P1;
  packet.header.uid = getHWID();
  packet.sample = 0;

  // "Subscribe" to task notifications for new vaisala messages
  vaisala_task_to_notify(xTaskGetCurrentTaskHandle());

  for (;;) {
    BaseType_t received_vaisala_data =
        xTaskNotifyWait(pdFALSE, UINT32_MAX, NULL, max_message_interval_ms);
    if (received_vaisala_data == pdPASS) {
      vaisala_reading_t *latest = vaisala_get_latest();
      packet.temperature = (int16_t)(latest->temp_air * 100);
      packet.humidity = (int16_t)(latest->rh * 100);
      packet.pressure = (int16_t)(latest->pressure * 100 - 100000.0);
      packet.alt_temperature = (int16_t)(latest->temp_internal * 100);
      // Store wind speed in kph * 100
      packet.wind_speed = (uint16_t)(latest->speed_avg * 360);
      packet.gust_speed = (uint16_t)(latest->speed_max * 360);

      packet.wind_dir_deg = latest->direction_avg;
      // packet.rain = windRainGetRain()/2794;
    } else {
      packet.temperature = -27300;
      packet.humidity = 0;
      packet.pressure = INT16_MIN;
      packet.alt_temperature = -27300;
      packet.rain = 0;
    }

    float fVal;
    if (sensorGetAvg(&vBatt, &fVal)) {
      packet.battery = (uint16_t)fVal;
      sensorClearSamples(&vBatt);
    } else {
      packet.battery = 0;
    }

    if (sensorGetAvg(&vSolar, &fVal)) {
      packet.solar_panel = (uint16_t)fVal;
      sensorClearSamples(&vSolar);
    } else {
      packet.solar_panel = 0;
    }

    packetTx(sizeof(packet), &packet, packetTxFn);

    packet.sample++;
  }
}

static void sensorsTask(void *pvParameters) {
  (void)pvParameters;

  LL_GPIO_ResetOutputPin(RADIO_NRST_GPIO_Port, RADIO_NRST_Pin);

  for (;;) {
    // int16_t sTemperature, sHumidity;
    // ulRval = ulSht3xRead(&hi2c1, SHT3x_ADDR, &sTemperature, &sHumidity);
    // if(ulRval == 0) {
    //     sensorAddSample(&shtTemperature, sTemperature);
    //     sensorAddSample(&shtHumidity, sHumidity);
    // }

    // float fTemperature, fPressure;
    // ulRval = dps368_measure_temp_once(&fTemperature);
    // if (ulRval == 0) {
    //     sensorAddSample(&dpsTemperature, fTemperature);
    // } else {
    //     // printf("Error reading DPS368 Temperature (%d)\n", ulRval);
    // }
    // ulRval = dps368_measure_pressure_once(&fPressure);
    // if (ulRval == 0) {
    //     sensorAddSample(&dpsPressure, (fPressure - 100000.0));
    // } else  {
    //     // printf("Error reading DPS368 Pressure (%d)\n", ulRval);
    // }
    // xIOI2cDeInit(&hi2c1);

    xIOAdcInit(&hadc1);
    sensorAddSample(&vBatt, prvAdcGetSampleMv(ADC_CHANNEL_5) * 2);
    sensorAddSample(&vSolar, prvAdcGetSampleMv(ADC_CHANNEL_6) * 2);
    xIOAdcDeInit(&hadc1);

    vTaskDelay(1000);
  }
}

extern SPI_HandleTypeDef hspi1;

void spiSetupFn() { MX_SPI1_Init(); }

void spiTeardownFn() { HAL_SPI_DeInit(&hspi1); }

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

static void watchdogTask(void *pvParameters) {
  (void)pvParameters;

  for (;;) {
    HAL_IWDG_Refresh(&hiwdg);
    vTaskDelay(10);
  }
}

int main(void) {
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_LPUART1_UART_Init();
  MX_IWDG_Init();

  memfault_platform_boot();

  BaseType_t xRval = xTaskCreate(mainTask, "main", 512, NULL, 3, NULL);

  configASSERT(xRval == pdTRUE);

  loraRadioInit(&loraConfig);

  xRval = xTaskCreate(sensorsTask, "sensors", 512, NULL, 2, NULL);

  configASSERT(xRval == pdTRUE);

  xRval = xTaskCreate(watchdogTask, "iwdg", 128, NULL, 0, NULL);
  configASSERT(xRval == pdTRUE);

  serialInit(LPUART1);
  serialSetRxByteCallback(vaisala_process_byte);

  lpmInit();
  // lpmEnable();

  useFreeRTOSTick = true;
  HAL_SuspendTick();

  vTaskStartScheduler();

  __builtin_unreachable();
}
