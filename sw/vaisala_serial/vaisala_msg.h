#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define VCRC_LEN (3)

typedef struct {
  char chr[VCRC_LEN];
  char nul;  // In case we want to treat it like a string
} vcrc_t;

vcrc_t vcrc_from_crc16(uint16_t crc16);
uint16_t crc_from_vcrc(const char *crc);
bool message_is_valid(const char *message, size_t len);

// int main() {
//   printf("Vaisala ASCII CRC test\n");
//   printf("Test string: \"%.*s\"\n", (int)(strlen(message) - 3), message);

//   printf("Message is valid: %d\n", message_is_valid(message,
//   strlen(message))); crc_t crc = crc_init(); crc = crc_update(crc, message,
//   (int)strlen(message) - 3); crc = crc_finalize(crc);

//   vcrc_t vcrc = vcrc_from_crc16(crc);
//   printf("crc: %04X (%s) Target is %s\n", (uint16_t)crc, vcrc.chr,
//          &message[strlen(message) - 3]);

//   printf("VCRC: %04X\n", crc_from_vcrc(&message[strlen(message) - 3]));
// }
