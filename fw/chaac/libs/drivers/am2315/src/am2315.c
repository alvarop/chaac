#include <stdint.h>
#include <bsp.h>
#include <os/os.h>
#include <hal/hal_i2c.h>
#include <stdio.h>

#define AM2315_ADDR             0x5C
#define AM2315_REG_READ         0x03

int32_t am2315_init() {

    return 0;
}

int32_t am2315_read(float *temperature, float *humidity) {

    struct hal_i2c_master_data i2c_data = {
        .address = AM2315_ADDR
    };

    uint8_t wbuff[3] = {0};
    uint8_t rbuff[8] = {0};

    int32_t rval = 0;

    do {
        hal_i2c_master_probe(0, AM2315_ADDR, 10);

        // Read temperature and humidity register
        // send request to AM2315
        wbuff[0] = AM2315_REG_READ;
        wbuff[1] = 0x00;  // read from adr 0x00
        wbuff[2] = 0x04;  // read 4 bytes

        i2c_data.len = 3;
        i2c_data.buffer = wbuff;
        rval = hal_i2c_master_write(0, &i2c_data, 10, 1);

        if(rval != 0) {
            break;
        }

        // Wait for data to be ready
        os_time_delay(2);

        i2c_data.len = 8;
        i2c_data.buffer = rbuff;
        rval = hal_i2c_master_read(0, &i2c_data, 10, 1);

        if(rval != 0) {
            break;
        }

        if(humidity != NULL) {
            *humidity = (rbuff[2] * 256.0 + rbuff[3])/10.0;
        }

        if(temperature != NULL) {
            *temperature = ((rbuff[4] & 0x7F) * 256.0 + rbuff[5])/10;
            if(rbuff[4] >> 7) {
                *temperature *= -1.0;
            }
        }
    } while (0);

    return rval;
}

