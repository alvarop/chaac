#include <stdint.h>
#include <bsp.h>
#include <hal/hal_i2c.h>

#define BMP280_ADDR             (0x76)
#define BMP280_ADDR_ALT         (0x77)
#define BMP280_DIG_T1_ADDR      (0x88)
#define BMP280_CHIP_ID_ADDR     (0xD0)
#define BMP280_SOFT_RESET_ADDR  (0xE0)
#define BMP280_STATUS_ADDR      (0xF3)
#define BMP280_CTRL_MEAS_ADDR   (0xF4)
#define BMP280_CONFIG_ADDR      (0xF5)
#define BMP280_PRES_ADDR        (0xF7)
#define BMP280_TEMP_ADDR        (0xFA)

#define BMP280_OS_NONE          (0x00)
#define BMP280_OS_1X            (0x01)
#define BMP280_OS_2X            (0x02)
#define BMP280_OS_4X            (0x03)
#define BMP280_OS_8X            (0x04)
#define BMP280_OS_16X           (0x05)

#define BMP280_SLEEP_MODE       (0x00)
#define BMP280_FORCED_MODE      (0x01)
#define BMP280_NORMAL_MODE      (0x03)

#define BMP280_ODR_0_5_MS       (0x00)
#define BMP280_ODR_62_5_MS      (0x01)
#define BMP280_ODR_125_MS       (0x02)
#define BMP280_ODR_250_MS       (0x03)
#define BMP280_ODR_500_MS       (0x04)
#define BMP280_ODR_1000_MS      (0x05)
#define BMP280_ODR_2000_MS      (0x06)
#define BMP280_ODR_4000_MS      (0x07)

#define BMP280_FILTER_OFF       (0x00)
#define BMP280_FILTER_COEFF_2   (0x01)
#define BMP280_FILTER_COEFF_4   (0x02)
#define BMP280_FILTER_COEFF_8   (0x03)
#define BMP280_FILTER_COEFF_16  (0x04)

typedef struct {
    uint16_t t1;
    int16_t t2;
    int16_t t3;
    uint16_t p1;
    int16_t p2;
    int16_t p3;
    int16_t p4;
    int16_t p5;
    int16_t p6;
    int16_t p7;
    int16_t p8;
    int16_t p9;
} __attribute__((packed)) bmp280_cal_t;

static bmp280_cal_t cal;

int32_t bmp280_init() {
    struct hal_i2c_master_data i2c_data = {
        .address = BMP280_ADDR
    };
    uint8_t cmd[4];
    int32_t rval = 0;

    do {
        cmd[0] = BMP280_CTRL_MEAS_ADDR;
        cmd[1] = (BMP280_OS_1X << 2) |
                  (BMP280_OS_1X << 5) |
                  BMP280_SLEEP_MODE; // 1x oversampling

        i2c_data.len = 2;
        i2c_data.buffer = cmd;
        rval = hal_i2c_master_write(0, &i2c_data, 10, 1);
        if(rval != 0) {
            break;
        }

        cmd[0] = BMP280_CONFIG_ADDR;
        cmd[1] = (BMP280_FILTER_OFF << 2) |
                  (BMP280_ODR_1000_MS << 5);

        i2c_data.len = 2;
        i2c_data.buffer = cmd;
        rval = hal_i2c_master_write(0, &i2c_data, 10, 1);
        if(rval != 0) {
            break;
        }

        cmd[0] = BMP280_DIG_T1_ADDR;

        i2c_data.len = 1;
        i2c_data.buffer = cmd;
        hal_i2c_master_write(0, &i2c_data, 10, 1);

        i2c_data.len = sizeof(bmp280_cal_t);
        i2c_data.buffer = (uint8_t *)&cal;
        rval = hal_i2c_master_read(0, &i2c_data, 10, 1);

        if(rval != 0) {
            break;
        }

    } while(0);

    return rval;
}

int32_t bmp280_start_forced_measurement() {
    struct hal_i2c_master_data i2c_data = {
        .address = BMP280_ADDR
    };
    uint8_t cmd[4];
    int32_t rval = 0;

    do {
        cmd[0] = BMP280_CTRL_MEAS_ADDR;
        cmd[1] = (BMP280_OS_1X << 2) |
                  (BMP280_OS_1X << 5) |
                  BMP280_FORCED_MODE;

        i2c_data.len = 2;
        i2c_data.buffer = cmd;
        rval = hal_i2c_master_write(0, &i2c_data, 10, 1);
        if(rval != 0) {
            break;
        }
    } while(0);

    // Made up number, only there so it won't hang on failure
    int16_t timeout = 8;
    while (!rval && (cmd[0] & 0x08) && (timeout > 0)) {
        cmd[0] = BMP280_STATUS_ADDR;
        i2c_data.len = 1;
        i2c_data.buffer = cmd;
        hal_i2c_master_write(0, &i2c_data, 10, 1);

        i2c_data.len = 1;
        i2c_data.buffer = cmd;
        rval = hal_i2c_master_read(0, &i2c_data, 10, 1);
        if(rval != 0) {
            break;
        }

        timeout--;
    }

    if (timeout == 0) {
        rval = -1;
    }

    return rval;
}

int32_t bmp280_read(float *temperature, float *pressure) {
    struct hal_i2c_master_data i2c_data = {
        .address = BMP280_ADDR
    };
    uint32_t temp_raw;
    uint32_t press_raw;
    int32_t t_fine;
    float var1, var2;
    uint8_t cmd[6];
    int32_t rval = 0;

    do {
        // Read both temperature and pressure registers
        cmd[0] = BMP280_PRES_ADDR;
        i2c_data.len = 1;
        i2c_data.buffer = cmd;
        hal_i2c_master_write(0, &i2c_data, 10, 1);

        i2c_data.len = 6;
        i2c_data.buffer = cmd;
        rval = hal_i2c_master_read(0, &i2c_data, 10, 1);
        if(rval != 0) {
            break;
        }

        press_raw = (cmd[0] << 12) | (cmd[1] << 4) | (cmd[2] >> 4);
        temp_raw = (cmd[3] << 12) | (cmd[4] << 4) | (cmd[5] >> 4);

        var1 = (((float) temp_raw) / 16384.0 - ((float) cal.t1) / 1024.0) * ((float) cal.t2);
        var2 = ((((float) temp_raw) / 131072.0 - ((float) cal.t1) / 8192.0) * (((float) temp_raw) / 131072.0 - ((float) cal.t1) / 8192.0)) * ((float) cal.t3);
        t_fine = (int32_t) (var1 + var2);

        if(temperature != NULL) {
            *temperature = (var1 + var2) / 5120.0;
        }

        var1 = ((float) t_fine / 2.0) - 64000.0;
        var2 = var1 * var1 * ((float) cal.p6) / 32768.0;
        var2 = var2 + var1 * ((float) cal.p5) * 2.0;
        var2 = (var2 / 4.0) + (((float) cal.p4) * 65536.0);
        var1 = (((float) cal.p3) * var1 * var1 / 524288.0 + ((float) cal.p2) * var1) / 524288.0;
        var1 = (1.0 + var1 / 32768.0) * ((float) cal.p1);

        if(pressure != NULL) {
            *pressure = 1048576.0 - (float) press_raw;

            if (var1 != 0) {
              *pressure = (*pressure - (var2 / 4096.0)) * 6250.0 / var1;
              var1 = ((float) cal.p9) * *pressure * *pressure / 2147483648.0;
              var2 = *pressure * ((float) cal.p8) / 32768.0;
              *pressure = *pressure + (var1 + var2 + ((float) cal.p7)) / 16.0;
            } else {
              *pressure = 0;
            }
        }
    } while(0);

    return rval;
}
