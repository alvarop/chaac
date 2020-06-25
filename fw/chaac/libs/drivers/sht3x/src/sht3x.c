#include <stdint.h>
#include <console/console.h>
#include <assert.h>
#include <hal/hal_i2c.h>
#include <sht3x/sht3x.h>
#include <os/os_cputime.h>

#define RD_TH_HIGH_HOLD_H 0x2c
#define RD_TH_HIGH_HOLD_L 0x06
#define RD_TH_MED_HOLD_H 0x2c
#define RD_TH_MED_HOLD_L 0x0D
#define RD_TH_LOW_HOLD_H 0x2c
#define RD_TH_LOW_HOLD_L 0x10
#define RD_TH_HIGH_NOHOLD_H 0x24
#define RD_TH_HIGH_NOHOLD_L 0x00
#define RD_TH_MED_NOHOLD_H 0x24
#define RD_TH_MED_NOHOLD_L 0x0B
#define RD_TH_LOW_NOHOLD_H 0x24
#define RD_TH_LOW_NOHOLD_L 0x16
#define HEATER_ENABLE_H 0x30
#define HEATER_ENABLE_L 0x6D
#define HEATER_DISABLE_H 0x30
#define HEATER_DISABLE_L 0x66
#define RD_STATUS_H 0xF3
#define RD_STATUS_L 0x2D
#define CLR_STATUS_H 0x30
#define CLR_STATUS_L 0x41
#define RESET_H 0x30
#define RESET_L 0xA2

#define I2C_PORT (0)

static struct hal_i2c_master_data i2c_data = {
    .address = SHT3x_ADDR
};

int32_t sht3x_init(uint8_t addr) {
    int32_t rval = 0;
    int16_t status = 0;

    do {
        rval = sht3x_reset(addr);
        if(rval != 0) {
            console_printf("SHT could not reset\n");
            break;
        }

        // Device requires 500us after soft reset
        os_cputime_delay_usecs(500);

        rval = sht3x_status(addr, &status);
        if(rval != 0) {
            console_printf("SHT could not read status\n");
            break;
        }

        if((status & 0x10) == 0) {
            console_printf("SHT invalid status\n");
            rval = SHT3x_ERR;
            break;
        }
    } while (0);

    return rval;
}

int32_t sht3x_status(uint8_t addr, int16_t *status) {
    int32_t rval = 0;
    uint8_t rBuff[2];
    uint8_t wBuff[2] = {RD_STATUS_H, RD_STATUS_L};

    do {
        i2c_data.len = sizeof(wBuff);
        i2c_data.buffer = wBuff;
        rval = hal_i2c_master_write(0, &i2c_data, 10, 1);

        if(rval != 0) {
            break;
        }

        i2c_data.len = sizeof(rBuff);
        i2c_data.buffer = rBuff;
        rval = hal_i2c_master_read(0, &i2c_data, 10, 1);

        if(rval != 0) {
            break;
        }

        if(status != NULL) {
            *status = (rBuff[0] << 8) | rBuff[1];
        }
    } while(0);

    return rval;
}

int32_t sht3x_reset(uint8_t addr) {
    int32_t rval = 0;
    uint8_t wBuff[2] = {RESET_H, RESET_L};
    struct hal_i2c_master_data i2c_data = {
        .address = SHT3x_ADDR
    };

    i2c_data.len = sizeof(wBuff);
    i2c_data.buffer = wBuff;
    rval = hal_i2c_master_write(0, &i2c_data, 10, 1);

    return rval;
}

int32_t sht3x_heater(uint8_t addr, bool enable) {
    int32_t rval = 0;
    uint8_t wBuff[2];

    struct hal_i2c_master_data i2c_data = {
        .address = SHT3x_ADDR
    };

    if (enable) {
        wBuff[0] = HEATER_ENABLE_H;
        wBuff[1] = HEATER_ENABLE_L;
    } else {
        wBuff[0] = HEATER_DISABLE_H;
        wBuff[1] = HEATER_DISABLE_L;
    }

    i2c_data.len = sizeof(wBuff);
    i2c_data.buffer = wBuff;
    rval = hal_i2c_master_write(0, &i2c_data, 10, 1);

    return rval;
}

int32_t sht3x_read(uint8_t addr, int16_t *temperature, int16_t *humidity) {
    int32_t rval = 0;
    uint8_t rBuff[6];
    uint8_t wBuff[2] = {RD_TH_HIGH_NOHOLD_H, RD_TH_HIGH_NOHOLD_L};

    do {
        // Send read command
        i2c_data.len = sizeof(wBuff);
        i2c_data.buffer = wBuff;
        rval = hal_i2c_master_write(0, &i2c_data, 10, 1);

        if(rval != 0) {
            break;
        }

        // Wait 15ms for sample
        os_cputime_delay_usecs(15000);

        // Read back measurements
        i2c_data.len = sizeof(rBuff);
        i2c_data.buffer = rBuff;
        rval = hal_i2c_master_read(0, &i2c_data, 10, 1);

        if(rval != 0) {
            break;
        }

        if(temperature != NULL) {
            uint32_t temp;
            // Get temperature * 100
            temp = (rBuff[0] << 8) | rBuff[1];
            temp = -4500 + (temp * 17500)/0xFFFF;
            *temperature = (uint16_t)temp;
        }

        if(humidity != NULL) {
            uint32_t rh;
            // Get humidity * 100
            rh = (rBuff[3] << 8) | rBuff[4];
            rh = (10000 * rh)/0xFFFF;
            *humidity = (uint16_t)rh;
        }

    } while(0);

    return rval;
}
