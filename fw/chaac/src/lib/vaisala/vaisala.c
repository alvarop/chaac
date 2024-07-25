#include "vaisala.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "vaisala_crc.h"

// Convert CRC-16 to 3 ascii characters
// See C.1 Encoding the CRC as ASCII Characters
// in M211840EN-D (WXT530 User Guide)
vcrc_t vcrc_from_crc16(uint16_t crc16) {
  vcrc_t vcrc;
  vcrc.chr[0] = (0x40 | (crc16 >> 12));
  vcrc.chr[1] = (0x40 | ((crc16 >> 6) & 0x3F));
  vcrc.chr[2] = (0x40 | (crc16 & 0x3F));
  vcrc.nul = 0;
  return vcrc;
}

// Convert 3 ascii characters to a CRC16 (reverse of vcrc_from_crc16)
uint16_t crc_from_vcrc(const char *crc) {
  return ((crc[0] - 0x40) << 12) + ((crc[1] - 0x40) << 6) + (crc[2] - 0x40);
}

bool message_is_valid(const char *message, size_t len) {
  if (!message) {
    return false;
  }

  crc_t computed_crc = vaisala_crc_init();
  computed_crc = vaisala_crc_update(computed_crc, message, len - VCRC_LEN);
  computed_crc = vaisala_crc_finalize(computed_crc);

  uint16_t message_crc = crc_from_vcrc(&message[len - VCRC_LEN]);

  return message_crc == computed_crc;
}

typedef enum {
  kCodeDn = 0x6E44,
  kCodeDm = 0x6D44,
  kCodeDx = 0x7844,
  kCodeSn = 0x6E53,
  kCodeSm = 0x6D53,
  kCodeSx = 0x7853,
  kCodeTa = 0x6154,
  kCodeTp = 0x7054,
  kCodeUa = 0x6155,
  kCodePa = 0x6150,
  kCodeRc = 0x6352,
  kCodeRd = 0x6452,
  kCodeRi = 0x6952,
  kCodeHc = 0x6348,
  kCodeHd = 0x6448,
  kCodeHi = 0x6948,
  kCodeRp = 0x7052,
  kCodeHp = 0x7048,
  kCodeTh = 0x6854,
} vaisala_code_t;

typedef struct {
  union {
    char ascii[2];
    uint16_t int16;
  } code;
  char equals;
  char value[0];
} __attribute__((__packed__)) vaisala_ascii_item_t;

uint32_t fake_pow32(uint32_t x, uint32_t y) {
  if (y == 0) {
    return 1;
  }
  uint32_t val = x;
  for (uint32_t idx = 1; idx < y; idx++) {
    val *= x;
  }

  return val;
}

float fake_strtof(const char *str) {
  float val;
  char *dot;

  int num = strtol(str, &dot, 10);

  val = (float)num;
  if (*dot == '.') {
    char *end;
    int fraction = strtol(&dot[1], &end, 10);

    uint32_t div = fake_pow32(10, (int)(end - &dot[1]));

    if (num >= 0) {
      val += (float)fraction / div;
    } else {
      val -= (float)fraction / div;
    }
  }

  return val;
}

static void _parse_item(vaisala_ascii_item_t *item) {
  switch ((vaisala_code_t)item->code.int16) {
    case kCodeDn: {
      int direction = strtol(item->value, NULL, 10);
      printf("Wind direction minimum = %d degrees\n", direction);
      break;
    }
    case kCodeDm: {
      int direction = strtol(item->value, NULL, 10);
      printf("Wind direction average = %d degrees\n", direction);
      break;
    }
    case kCodeDx: {
      int direction = strtol(item->value, NULL, 10);
      printf("Wind direction maximum = %d degrees\n", direction);
      break;
    }
    case kCodeSn: {
      float speed = fake_strtof(item->value);
      printf("Wind speed minimum = %0.3f m/s\n", speed);
      break;
    }
    case kCodeSm: {
      float speed = fake_strtof(item->value);
      printf("Wind speed average = %0.3f m/s\n", speed);
      break;
    }
    case kCodeSx: {
      float speed = fake_strtof(item->value);
      printf("Wind speed maximum = %0.3f m/s\n", speed);
      break;
    }
    case kCodeTa: {
      float temp = fake_strtof(item->value);
      printf("Air Temperature = %0.1f C\n", temp);
      break;
    }
    case kCodeTp: {
      float temp = fake_strtof(item->value);
      printf("Internal Temperature = %0.1f C\n", temp);
      break;
    }
    case kCodeUa: {
      float rh = fake_strtof(item->value);
      printf("Relative humidity = %0.1f %%RH\n", rh);
      break;
    }
    case kCodePa: {
      float pressure = fake_strtof(item->value);
      printf("Air pressure = %0.1f hPa\n", pressure);
      break;
    }
    case kCodeRc: {
      printf("kCodeRc = %s\n", item->value);
      break;
    }
    case kCodeRd: {
      printf("kCodeRd = %s\n", item->value);
      break;
    }
    case kCodeRi: {
      printf("kCodeRi = %s\n", item->value);
      break;
    }
    case kCodeHc: {
      printf("kCodeHc = %s\n", item->value);
      break;
    }
    case kCodeHd: {
      printf("kCodeHd = %s\n", item->value);
      break;
    }
    case kCodeHi: {
      printf("kCodeHi = %s\n", item->value);
      break;
    }
    case kCodeRp: {
      printf("kCodeRp = %s\n", item->value);
      break;
    }
    case kCodeHp: {
      printf("kCodeHp = %s\n", item->value);
      break;
    }
    case kCodeTh: {
      printf("kCodeTh = %s\n", item->value);
      break;
    }
    default: {
      break;
    };
  }
}

void vaisala_parse_msg(char *buff, size_t len) {
  if (message_is_valid(buff, len)) {
    fake_strtof("-1.3234");
    fake_strtof("-123.2");
    fake_strtof("-123123");
    fake_strtof("12345.09876");

    // Step back from CRC
    len -= 3;
    printf("%.*s\n", (int)len, buff);

    // We've already verified the CRC, put a null character
    // so the last token doesn't include it
    buff[len] = 0;

    char *token = strtok(buff, ",");
    while (token) {
      vaisala_ascii_item_t *item = (vaisala_ascii_item_t *)token;
      if (item->equals == '=') {
        _parse_item(item);
      }

      token = strtok(NULL, ",");
    }

  } else {
    printf("Invalid mesage: %.*s\n", (int)len, buff);
  }
}