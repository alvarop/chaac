#include "vaisala_msg.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "crc.h"

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

  crc_t computed_crc = crc_init();
  computed_crc = crc_update(computed_crc, message, len - VCRC_LEN);
  computed_crc = crc_finalize(computed_crc);

  uint16_t message_crc = crc_from_vcrc(&message[len - VCRC_LEN]);

  return message_crc == computed_crc;
}
