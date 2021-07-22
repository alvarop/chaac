#ifndef __DPS368_H__
#define __DPS368_H__

#include <stdint.h>
#include <stdbool.h>

#define DPS368_ADDR (0x76 << 1)
#define DPS368_ALT_ADDR (0x77 << 1)

int32_t dps368_init();

int16_t dps368_standby(void);
int16_t dps368_set_op_mode(uint8_t opMode);
int16_t dps368_config_temp(uint8_t tempMr, uint8_t tempOsr);
int16_t dps368_config_pressure(uint8_t prsMr, uint8_t prsOsr);
int16_t dps368_enable_fifo();
int16_t dps368_disable_fifo();
int16_t dps368_flush_fifo(); 
int16_t dps368_measure_temp_once(float *result);
int16_t dps368_measure_pressure_once(float *result);
#endif
