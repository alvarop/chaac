#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#define VCRC_LEN (3)

typedef struct {
  char chr[VCRC_LEN];
  char nul;  // In case we want to treat it like a string
} vcrc_t;

typedef struct {
  int direction_min;
  int direction_avg;
  int direction_max;
  float speed_min;
  float speed_avg;
  float speed_max;
  float temp_air;
  float temp_internal;
  float rh;
  float pressure;
  float rain_acc;
  float rain_duration;
  float rain_intensity;
  float hail_acc;
  float hail_duration;
  float hail_intensity;
  float rain_peak_intensity;
  float hail_peak_intensity;
  float temp_heating;
} vaisala_reading_t;

vcrc_t vcrc_from_crc16(uint16_t crc16);
uint16_t crc_from_vcrc(const uint8_t *crc);
bool message_is_valid(const uint8_t *message, size_t len);
bool vaisala_parse_msg(uint8_t *buff, size_t len, vaisala_reading_t *reading);
void vaisala_process_byte(uint8_t byte);
void vaisala_task_to_notify(TaskHandle_t task_to_notify);
vaisala_reading_t *vaisala_get_latest();
