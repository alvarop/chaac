#include "windrain.h"
#include "adc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "printf.h"

typedef struct {
    uint16_t voltage;
    uint16_t degrees;
    wind_dir_t direction;
} WindDirLUT_t;

TaskHandle_t windRainTaskHandle;

static uint32_t windSpeedTicks;
static uint32_t windGustTicks;
static uint32_t lastSpeedTicks;
static uint32_t lastGustTicks;
static volatile uint32_t maxGust;
static uint32_t rainTicks;

static TimerHandle_t gustTimer;

#define NOTIFY_RAIN (1 << 0)
#define NOTIFY_GUST (1 << 1)
static volatile uint32_t windRainNotifyFlags = 0;

static const WindDirLUT_t windDirLUT[] = {
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

void windRainWindSpeedIrq() {
    // TODO - add debounce
    windSpeedTicks++;
    windGustTicks++;
}

void windRainRainIrq() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if(windRainTaskHandle != NULL) {
        windRainNotifyFlags |= NOTIFY_RAIN;
        vTaskNotifyGiveFromISR(windRainTaskHandle, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void gustTimerCallback(TimerHandle_t timer) {
  (void)timer;
  taskENTER_CRITICAL();
  windRainNotifyFlags |= NOTIFY_GUST;
  taskEXIT_CRITICAL();

  xTaskNotifyGive(windRainTaskHandle);
}

uint32_t windRainGetRain() {
    // 0.2794 mm of rain per tick
    return rainTicks * 2794;
}

void windRainClearRain() {
    rainTicks = 0;
}

static uint32_t calcWindSpeed(uint32_t ticks, uint32_t timeDiff) {
    uint32_t windSpeed = 0;
    if (timeDiff > 0) {
        // 1 tick/s is equivalent to 2.4kph
        windSpeed = ticks * 2400 * configTICK_RATE_HZ / (timeDiff);
    }

    return windSpeed;
}

uint32_t windRainGetSpeed() {
    uint32_t timeDiff = xTaskGetTickCount() - lastSpeedTicks;
    uint32_t windSpeed = calcWindSpeed(windSpeedTicks, timeDiff);

    windSpeedTicks = 0;
    lastSpeedTicks = xTaskGetTickCount();

    return windSpeed;
}

uint32_t windRainGetGust() {
    uint32_t gustSpeed = maxGust;

    taskENTER_CRITICAL();
    maxGust = 0;
    taskEXIT_CRITICAL();

    return gustSpeed;
}

static const WindDirLUT_t *getDir(int32_t dirMv) {

    const WindDirLUT_t *pDir = NULL;
    for (uint8_t dir = 0; dir < sizeof(windDirLUT)/sizeof(wind_dir_t); dir++) {
        if (dirMv < windDirLUT[dir].voltage) {
            pDir = &windDirLUT[dir];
            break;
        }
    }

    return pDir;
}

int16_t windRainGetDirDegrees(int32_t dirMv) {
    const WindDirLUT_t *pDir = getDir(dirMv);

    if (pDir != NULL) {
        return pDir->degrees;
    }
    return -1;
}

wind_dir_t windRainGetDir(int32_t dirMv) {
    const WindDirLUT_t *pDir = getDir(dirMv);

    if (pDir != NULL) {
        return pDir->direction;
    }
    return WIND_INVALID;
 }


static void windRainTask( void *pvParameters ) {
    (void)pvParameters;
    static uint32_t lastRainTime = 0; // Used for debounce

    gustTimer = xTimerCreate(
        "gust",
        pdMS_TO_TICKS(3 * 1000),
        pdTRUE, // Enable auto-reload
        NULL,
    gustTimerCallback);
    configASSERT(gustTimer);
    configASSERT(xTimerStart(gustTimer, 10));

    for(;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        taskENTER_CRITICAL();
        uint32_t flags = windRainNotifyFlags;
        windRainNotifyFlags = 0;
        taskEXIT_CRITICAL();

        if(flags & NOTIFY_RAIN) {
            // Use OS time for debounce (1/1000 ~= 1ms) configTICK_RATE_HZ
            if(xTaskGetTickCount() != lastRainTime) {
                lastRainTime = xTaskGetTickCount();
                rainTicks++;
            }
        }

        if(flags & NOTIFY_GUST) {
            // Wind gust calculation
            uint32_t timeDiff = xTaskGetTickCount() - lastGustTicks;
            uint32_t gustSpeed = calcWindSpeed(windGustTicks, timeDiff);

            windGustTicks = 0;
            lastGustTicks = xTaskGetTickCount();

            taskENTER_CRITICAL();
            if(gustSpeed > maxGust) {
                maxGust = gustSpeed;
            }
            taskEXIT_CRITICAL();
        }
    }
}


void windRainInit() {
    BaseType_t xRval = xTaskCreate(
        windRainTask,
        "windRain",
        512,
        NULL,
        3,
        &windRainTaskHandle);

    configASSERT(xRval == pdTRUE);
    lastSpeedTicks= 0;

    // Dummy read to reset the speed/gust timers
    windRainGetSpeed();
    windRainGetGust();
}

