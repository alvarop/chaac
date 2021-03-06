#include <assert.h>
#include <string.h>
#include <console/console.h>
#include <stdio.h>
#include <hal/hal_i2c.h>

#include <sysinit/sysinit.h>
#include <os/os.h>
#include <bsp/bsp.h>
#include <hal/hal_gpio.h>
#include <simple_adc/simple_adc.h>
#include <windrain/windrain.h>
#include <packet/packet.h>
#include <hal/hal_nvreg.h>
#include <sht3x/sht3x.h>

#include "chaac_packet.h"
#include "radio/radio.h"
#include "sx126x/sx126x.h"

#if MYNEWT_VAL(CHAAC_USE_BMP280)
#include <bmp280/bmp280.h>
#endif

#if MYNEWT_VAL(CHAAC_USE_DPS368)
#include <dps368/dps368.h>
#endif


#define RF_FREQUENCY 915000000

#define TX_OUTPUT_POWER 0

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


struct os_callout sample_callout;
static uint8_t sample_num;

static volatile uint16_t timestamp;

static weather_packet_v1p0_t packet;

static RadioEvents_t RadioEvents;

void OnTxDone( void )
{
    Radio.Sleep( );
    hal_gpio_write(LED1_PIN, 0);
    console_printf("TX Done\n");
}

void OnTxTimeout( void )
{
    Radio.Sleep( );  
    console_printf("TX Timeout\n");
    hal_gpio_write(LED1_PIN, 0);
}


int radio_init(void) {
    console_printf("Radio Init\n");

    RadioEvents.TxDone = OnTxDone;
    //RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    //RadioEvents.RxTimeout = OnRxTimeout;
    //RadioEvents.RxError = OnRxError;
    // RadioEvents.CadDone = OnCadDone;
    
    Radio.Init( &RadioEvents );

    Radio.SetChannel( RF_FREQUENCY );

    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

    Radio.Sleep();

    return 0;
}

void weather_init() {
    hal_gpio_init_out(LED1_PIN, 0);

    hal_gpio_init_out(WX_DIR_EN_PIN, WX_DIR_EN_OFF);

    int32_t rval = 0;

#if MYNEWT_VAL(CHAAC_USE_BMP280)
    rval = bmp280_init();
    if(rval) {
        console_printf("Error initializing BMP280 (%ld)\n", rval);
    }
#endif

#if MYNEWT_VAL(CHAAC_USE_DPS368)
    rval = dps368_init();
    if(rval) {
        console_printf("Error initializing DPS368 (%ld)\n", rval);
    }
#endif

    rval = sht3x_init(SHT3x_ADDR);
    if (rval != 0) {
        console_printf("Error intitializing SHT3x (%ld)\n", rval);
    }

    windrain_init();

    sample_num = 0;
}

const char * wind_dir_str[16] = {
    "N", "NNE", "NE", "ENE",
    "E", "ESE", "SE", "SSE",
    "S", "SSW", "SW", "WSW",
    "W", "WNW", "NW", "NNW"};

void weather_sample_fn(struct os_event *ev) {
    int32_t result = 0;
    int32_t rval;

    // schedule next event asap
    os_callout_reset(&sample_callout, OS_TICKS_PER_SEC * MYNEWT_VAL(CHAAC_SAMPLE_RATE_S));

    simple_adc_init();

    packet.sample = sample_num;

    hal_gpio_init_out(WX_DIR_EN_PIN, WX_DIR_EN_ON);
    // Measured rise time in WX_DIR pin ~1ms (with 0.1uF cap)
    // 5 ms delay is plenty to settle before measuring
    os_time_delay(os_time_ms_to_ticks32(5));

    rval = simple_adc_read_ch(BATT_ADC_CH, &result);
    if(rval) {
        console_printf("simple_adc_read_ch error %ld\n", rval);
    } else {
        packet.battery = result * 2;
        console_printf("BR: %ld\n", result);
        console_printf("B: %ld.%ld\n",
            (int32_t)(packet.battery/1000),
            (int32_t)((packet.battery-(int32_t)(packet.battery/1000)*1000)));
    }

#ifdef VSOLAR_ADC_CH
    rval = simple_adc_read_ch(VSOLAR_ADC_CH, &result);
    if(rval) {
        console_printf("simple_adc_read_ch error %ld\n", rval);
    } else {
        packet.solar_panel = (float)result * 2;
        console_printf("S: %ld.%ld\n",
            (int32_t)(packet.solar_panel/1000),
            (int32_t)((packet.solar_panel-(int32_t)(packet.solar_panel/1000)*1000)));
    }
#else
    packet.solar_panel = 0.0;
#endif

    {
        int16_t temperature, humidity;
        
        rval = sht3x_read(SHT3x_ADDR, &temperature, &humidity);
        if (rval) {
            // Set unrealistic values during error
            console_printf("Error reading from SHT3x (%ld)\n", rval);
            packet.temperature = -27300;
            packet.humidity = 0;
        } else {
            packet.temperature = (int16_t)(temperature);
            packet.humidity = (uint16_t)(humidity);

            console_printf("T: %ld.%02ld H:%ld.%02ld\n",
                (int32_t)(temperature/100),
                (uint32_t)(temperature-(uint32_t)(temperature/100)*100),
                (int32_t)(humidity/100),
                (int32_t)((humidity-(int32_t)(humidity/100)*100)));
        }
    }

#if MYNEWT_VAL(CHAAC_USE_BMP280)
    {
        float temperature, pressure;
        rval = bmp280_start_forced_measurement();
        if (rval) {
            console_printf("Error starting forced measurement for BMP280 (%ld)\n", rval);
        } else {
            rval = bmp280_read(&temperature, &pressure);
        }

        // pressure /= 100.0; // Convert to hPa
        if (rval) {
            console_printf("Error reading from BMP280 (%ld)\n", rval);
            // Set unrealistic values during error
            packet.pressure = INT16_MIN;
        } else {
            // Convert to hPa difference from 1000.00 hPa (to use int16_t)
            packet.pressure = (int16_t)(((pressure - 100000.0)));
            console_printf("P: %ld.%02ld T: %ld.%02ld\n",
                (int32_t)(pressure/100),
                (int32_t)((pressure/100-(int32_t)(pressure/100))*100),
                (int32_t)(temperature),
                (uint32_t)((temperature-(uint32_t)(temperature))*100));
        }
    }
#endif
#if MYNEWT_VAL(CHAAC_USE_DPS368)
    {
        float temperature, pressure;
        int16_t rval;

        rval = dps368_measure_temp_once(&temperature);
        if (rval != 0) {
            console_printf("Error reading DPS368 Temperature (%d)\n", rval);
        }
        rval = dps368_measure_pressure_once(&pressure);
        if (rval != 0) {
            console_printf("Error reading DPS368 Pressure (%d)\n", rval);
        }

        if (rval) {
            console_printf("Error reading from DPS368 (%d)\n", rval);
            // Set unrealistic values during error
            packet.pressure = INT16_MIN;
        } else {
            packet.pressure = (int16_t)(((pressure - 100000.0)));
            console_printf("P: %ld.%02ld T: %ld.%02ld\n",
                (int32_t)(pressure/100),
                (int32_t)((pressure/100-(int32_t)(pressure/100))*100),
                (int32_t)(temperature),
                (uint32_t)((temperature-(uint32_t)(temperature))*100));
       }

#
    }
#endif


    // Store rain by multiples of 0.2794mm
    packet.rain = windrain_get_rain()/2794;

    // Store wind speed in kph * 100
    packet.wind_speed = windrain_get_speed()/10;

    packet.wind_dir = windrain_get_dir();
    hal_gpio_init_out(WX_DIR_EN_PIN, WX_DIR_EN_OFF);

    console_printf("ws: %ld.%ld kph %s\n",
            (int32_t)(packet.wind_speed/100),
            (int32_t)((packet.wind_speed-(int32_t)(packet.wind_speed/100)*100)),
            wind_dir_str[packet.wind_dir]);
    console_printf("rain: %ld.%ld mm\n",
            (int32_t)(packet.rain*2794/10000),
            (int32_t)((packet.rain*2794-(int32_t)((packet.rain*2794)/10000))*10000));

    // Transmit packet over LoRa radio
    console_printf("Radiotx\n");
    hal_gpio_write(LED1_PIN, 1);
    Radio.Send((uint8_t *)&packet, sizeof(packet));

    sample_num++;

    // Turn off ADC for some small power savings
    simple_adc_uninit();

    windrain_clear_rain();
}

int main(int argc, char **argv) {
    int rc;

    sysinit();

    console_printf("Chaac v%d.%d\n", (CHAAC_HW_VERS >> 8), CHAAC_HW_VERS & 0xFF);
    console_printf("UID: %08lX\n", DEVICE_UID);
    console_printf("Sample period: %ds\n", MYNEWT_VAL(CHAAC_SAMPLE_RATE_S));

    radio_init();
    weather_init();

    packet.header.type = PACKET_TYPE_WEATHER_V1P0;
    packet.header.uid = DEVICE_UID;

    os_callout_init(&sample_callout, os_eventq_dflt_get(), weather_sample_fn, NULL);
    os_callout_reset(&sample_callout, OS_TICKS_PER_SEC * MYNEWT_VAL(CHAAC_SAMPLE_RATE_S));

    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    assert(0);
    return rc;
}
