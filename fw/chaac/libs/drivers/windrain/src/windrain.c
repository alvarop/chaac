#include <stdio.h>
#include <hal/hal_gpio.h>
#include <assert.h>
#include <simple_adc/simple_adc.h>
#include <bsp.h>
#include <os/os_time.h>

typedef struct {
    uint16_t voltage;
    uint16_t direction;
} wind_dir_t;

static uint32_t speed_ticks;
static int64_t last_speed_time_s;

static uint32_t rain_ticks;

// Lookup table to get wind direction from voltage
// Auto-generated with winddircalc.py
// VDD3.30= R_PU=4700 R_S=2000
static const wind_dir_t wind_dir_lut[] = {
    {1229, 1125},
    {1271, 675},
    {1337, 900},
    {1472, 1575},
    {1640, 1350},
    {1780, 2025},
    {1984, 1800},
    {2195, 225},
    {2407, 450},
    {2586, 2475},
    {2687, 2250},
    {2833, 3375},
    {2946, 0},
    {3033, 2925},
    {3131, 3150},
    {3384, 2700},
    {0 ,0}};


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

    rain_ticks = 0;

    return rain;
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

int16_t windrain_get_dir() {
    int32_t mv = 0;
    int16_t direction = -1;

    simple_adc_read_ch(15, &mv);

    printf("adc: %ld mv\n", mv);

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
}
