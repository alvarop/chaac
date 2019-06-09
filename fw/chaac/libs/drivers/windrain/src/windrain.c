#include <stdio.h>
#include <hal/hal_gpio.h>
#include <assert.h>
#include <simple_adc/simple_adc.h>
#include <bsp.h>
#include <os/os_time.h>
#include <windrain/windrain.h>
#include <console/console.h>

typedef struct {
    uint16_t voltage;
    uint16_t degrees;
    wind_dir_t direction;
} wind_dir_lut_t;

static uint32_t speed_ticks;
static int64_t last_speed_time_s;

static uint32_t rain_ticks;

// Lookup table to get wind direction from voltage
// Auto-generated with winddircalc.py (modified to add enums)
// VDD3.30= R_PU=4700 R_S=2000
static const wind_dir_lut_t wind_dir_lut[] = {
    {1229,  1125,    WIND_ESE},
    {1271,  675,     WIND_ENE},
    {1337,  900,     WIND_E},
    {1472,  1575,    WIND_SSE},
    {1640,  1350,    WIND_SE},
    {1780,  2025,    WIND_SSW},
    {1984,  1800,    WIND_S},
    {2195,  225,     WIND_NNE},
    {2407,  450,     WIND_NE},
    {2586,  2475,    WIND_WSW},
    {2687,  2250,    WIND_SW},
    {2833,  3375,    WIND_NNW},
    {2946,  0,       WIND_N},
    {3033,  2925,    WIND_WNW},
    {3131,  3150,    WIND_NW},
    {3384,  2700,    WIND_W},
    {0,     0,       WIND_N}};


static void wind_speed_irq(void *arg) {
    // TODO - add debounce
    speed_ticks++;
}

static void rain_irq(void *arg) {
    static os_time_t last_rain_time;
    // Use OS time for debounce (1/1000 ~= 1ms)
    if(os_time_get() != last_rain_time) {
        last_rain_time = os_time_get();
        rain_ticks++;
    }
}

uint32_t windrain_get_rain() {
    uint32_t rain = 0;

    // 0.2794 mm of rain per tick
    rain = rain_ticks * 2794;
    return rain;
}

void windrain_clear_rain() {
    rain_ticks = 0;
}

uint32_t windrain_get_speed() {
    uint32_t wind_speed;
    struct os_timeval tv;
    int rval = os_gettimeofday(&tv, NULL);
    assert(rval == 0);

    if (last_speed_time_s < tv.tv_sec) {
        // 1 tick/s is equivalent to 2.4kph
        wind_speed = speed_ticks * 2400 / (tv.tv_sec - last_speed_time_s);
    } else {
        wind_speed = 0;
    }

    speed_ticks = 0;
    last_speed_time_s = tv.tv_sec;

    return wind_speed;
}

int16_t windrain_get_dir_degrees() {
    int32_t mv = 0;
    int16_t degrees = -1;

    simple_adc_read_ch(WX_DIR_ADC_CH, &mv);
    console_printf("deg: %ld\n", mv);

    for (uint8_t dir = 0; dir < sizeof(wind_dir_lut)/sizeof(wind_dir_t); dir++) {
        if (mv < wind_dir_lut[dir].voltage) {
            degrees = wind_dir_lut[dir].degrees;
            break;
        }

        // TODO add invalid check
    }

    return degrees;
}

wind_dir_t windrain_get_dir() {
    int32_t mv = 0;
    wind_dir_t direction = -1;

    simple_adc_read_ch(WX_DIR_ADC_CH, &mv);
    console_printf("dir: %ld\n", mv);

    for (uint8_t dir = 0; dir < sizeof(wind_dir_lut)/sizeof(wind_dir_t); dir++) {
        if (mv < wind_dir_lut[dir].voltage) {
            direction = wind_dir_lut[dir].direction;
            break;
        }

        // TODO add invalid check
    }

    return direction;
}

void windrain_init() {

    last_speed_time_s = 0;

    hal_gpio_irq_init(WX_SPEED_PIN, wind_speed_irq, NULL,
        HAL_GPIO_TRIG_FALLING, HAL_GPIO_PULL_UP);

    hal_gpio_irq_init(WX_RAIN_PIN, rain_irq, NULL,
        HAL_GPIO_TRIG_FALLING, HAL_GPIO_PULL_UP);

    hal_gpio_irq_enable(WX_SPEED_PIN);
    hal_gpio_irq_enable(WX_RAIN_PIN);

    // Dummy read to reset the speed timer
    windrain_get_speed();
}
