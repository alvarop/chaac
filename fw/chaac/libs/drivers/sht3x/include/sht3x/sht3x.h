#ifndef __SHT3x_H__
#define __SHT3x_H__

#include <stdint.h>
#include <stdbool.h>

#define SHT3x_ADDR (0x44)
#define SHT3x_ALT_ADDR (0x45)

#define SHT3x_ERR (-14)

int32_t sht3x_init(uint8_t addr);
int32_t sht3x_status(uint8_t addr, int16_t *status);
int32_t sht3x_reset(uint8_t addr);
int32_t sht3x_heater(uint8_t addr, bool enable);
int32_t sht3x_read(uint8_t addr, int16_t *temp, int16_t *humidity);

#endif
