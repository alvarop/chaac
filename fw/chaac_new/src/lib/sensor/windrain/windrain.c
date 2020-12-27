#include "windrain.h"
#include "IOAdc.h"
#include "adc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "printf.h"

typedef struct {
    uint16_t voltage;
    uint16_t degrees;
    wind_dir_t direction;
} WindDirLUT_t;

static uint32_t ulWindSpeedTicks;
static uint32_t ulLastSpeedTimeS;

static uint32_t ulRainTicks;

static const WindDirLUT_t pxWindDirLUT[] = {
    { 455, 1125, WIND_ESE},
    { 488,  675, WIND_ENE},
    { 541,  900, WIND_E},
    { 655, 1575, WIND_SSE},
    { 802, 1350, WIND_SE},
    { 932, 2025, WIND_SSW},
    {1140, 1800, WIND_S},
    {1365,  225, WIND_NNE},
    {1622,  450, WIND_NE},
    {1848, 2475, WIND_WSW},
    {1988, 2250, WIND_SW},
    {2203, 3375, WIND_NNW},
    {2377,    0, WIND_N},
    {2520, 2925, WIND_WNW},
    {2688, 3150, WIND_NW},
    {2958, 2700, WIND_W},
    {   0,    0, WIND_N}};

void vWindRainWindSpeedIrq() {
    // TODO - add debounce
    ulWindSpeedTicks++;
}

void vWindRainRainIrq() {
    static uint32_t ulLastRainTime;

    // Use OS time for debounce (1/1000 ~= 1ms) configTICK_RATE_HZ
    if(xTaskGetTickCount() != ulLastRainTime) {
        ulLastRainTime = xTaskGetTickCount();
        ulRainTicks++;
    }
}

uint32_t ulWindRainGetRain() {
    // 0.2794 mm of rain per tick
    return ulRainTicks * 2794;
}

void vWindRainClearRain() {
    ulRainTicks = 0;
}

// TODO - add ulWindGetMaxSpeed for wind gusts
uint32_t ulWindRainGetSpeed() {
    uint32_t ulWindSpeed;
   
    int32_t ulTimeDiff = xTaskGetTickCount() - ulLastSpeedTimeS;
    if (ulTimeDiff > 0) {
        // 1 tick/s is equivalent to 2.4kph
        ulWindSpeed = ulWindSpeedTicks * 2400 * configTICK_RATE_HZ / (ulTimeDiff);
    } else {
        ulWindSpeed = 0;
    }

    ulWindSpeedTicks = 0;
    ulLastSpeedTimeS = xTaskGetTickCount();

    return ulWindSpeed;
}

static const WindDirLUT_t *prvGetDir() {
    int32_t lMv = 0;

    ADC_ChannelConfTypeDef xConfig = {0};

    xConfig.Rank = ADC_REGULAR_RANK_1;
    xConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
    xConfig.SingleDiff = ADC_SINGLE_ENDED;
    xConfig.OffsetNumber = ADC_OFFSET_NONE;
    xConfig.Offset = 0;

    xConfig.Channel = ADC_CHANNEL_5;
    xIOAdcChannelConfig(&hadc1, &xConfig);

    xIOAdcReadMv(&hadc1, &lMv);

    const WindDirLUT_t *pxDir = NULL;
    for (uint8_t dir = 0; dir < sizeof(pxWindDirLUT)/sizeof(wind_dir_t); dir++) {
        if (lMv < pxWindDirLUT[dir].voltage) {
            pxDir = &pxWindDirLUT[dir];
            break;
        }
    }

    return pxDir;
}

int16_t sWindRainGetDirDegrees() {
    const WindDirLUT_t *pxDir = prvGetDir();

    if (pxDir != NULL) {
        return pxDir->degrees;
    }
    return -1;
}

wind_dir_t xWindRainGetDir() {
    const WindDirLUT_t *pxDir = prvGetDir();

    if (pxDir != NULL) {
        return pxDir->direction;
    }
    return WIND_INVALID;
 }

void vWindRainInit() {

    ulLastSpeedTimeS= 0;

    // Dummy read to reset the speed timer
    ulWindRainGetSpeed();
}

