#include "stm32l4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "printf.h"
#include "io_i2c.h"
#include "debug.h"
#include "sht3x.h"
#include "dps368.h"
#include "adc.h"

extern I2C_HandleTypeDef hi2c1;

static float prvAdcGetSample(uint32_t ulChannel) {
    int32_t lResult = 0;

    ADC_ChannelConfTypeDef xConfig = {0};

    xConfig.Rank = ADC_REGULAR_RANK_1;
    xConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
    xConfig.SingleDiff = ADC_SINGLE_ENDED;
    xConfig.OffsetNumber = ADC_OFFSET_NONE;
    xConfig.Offset = 0;

    xConfig.Channel = ulChannel;
    if (HAL_ADC_ConfigChannel(&hadc1, &xConfig) != HAL_OK) {
        printf("Error configuring ADC channel\n");
    }
    
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    lResult = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);

    return ((float)lResult * 3.0)/(1 << 12);
}

static void prvMainTask( void *pvParameters ) {
    (void)pvParameters;
    
    LL_GPIO_ResetOutputPin(RADIO_NRST_GPIO_Port, RADIO_NRST_Pin);

#ifdef BUILD_DEBUG
    vDebugInit();
    printf("Chaac FW\n");
#endif
   
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

    // Calibrate ADC
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);

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

        printf("WDIR: %0.3f V\n", prvAdcGetSample(ADC_CHANNEL_5));
        printf("VSOLAR: %0.3f V\n", prvAdcGetSample(ADC_CHANNEL_6) * 2.0);
        printf("BATT: %0.3f V\n", prvAdcGetSample(ADC_CHANNEL_7) * 2.0);

        vTaskDelay(4925);
    }
}

__attribute__((noreturn)) void app_main() {
    BaseType_t xRval = xTaskCreate(
            prvMainTask,
            "main",
            512,
            NULL,
            tskIDLE_PRIORITY + 1,
            NULL);

    configASSERT(xRval == pdTRUE);

    vTaskStartScheduler();

    __builtin_unreachable();
}


