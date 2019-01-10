#ifndef __AM2315_H__
#define __AM2315_H__

#include <stdint.h>

int32_t am2315_init();
int32_t am2315_read(float *temperature, float *humidity);

#endif