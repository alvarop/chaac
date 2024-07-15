#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "vaisala_msg.h"

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: vaisala_serial port baudrate\n");
  }

  uint32_t baud_rate = strtoul(argv[2], NULL, 10);

  int serial_port = open(argv[1], O_RDWR);
  if (serial_port < 0) {
    printf("Error %i opening serial port: %s\n", errno, strerror(errno));
    return -1;
  }

  // Create new termios struct, we call it 'tty' for convention
  // No need for "= {0}" at the end as we'll immediately write the existing
  // config to this struct
  struct termios tty;

  // Read in existing settings, and handle any error
  // NOTE: This is important! POSIX states that the struct passed to tcsetattr()
  // must have been initialized with a call to tcgetattr() overwise behaviour
  // is undefined
  if (tcgetattr(serial_port, &tty) != 0) {
    printf("Error %d from tcgetattr: %s\n", errno, strerror(errno));
  }

  tty.c_cflag |= CS8;  // 8 bits per byte (most common)

  tty.c_cflag &= ~PARENB;  // Clear parity bit, disabling parity (most common)

  tty.c_cflag &= ~CSTOPB;  // Clear stop field, only one stop bit used in
                           // communication (most common)

  tty.c_cflag &=
      ~CRTSCTS;  // Disable RTS/CTS hardware flow control (most common)

  tty.c_cflag |=
      CREAD | CLOCAL;  // Turn on READ & ignore ctrl lines (CLOCAL = 1)

  tty.c_lflag &= ~ICANON;  // Disable canonical mode
  tty.c_lflag &= ~ECHO;    // Disable echo
  tty.c_lflag &= ~ECHOE;   // Disable erasure
  tty.c_lflag &= ~ECHONL;  // Disable new-line echo

  tty.c_lflag &= ~ISIG;  // Disable interpretation of INTR, QUIT and SUSP
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);  // Turn off s/w flow ctrl
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR |
                   ICRNL);  // Disable any special handling of received bytes
  tty.c_oflag &= ~OPOST;    // Prevent special interpretation of output bytes
                            // (e.g. newline chars)
  tty.c_oflag &=
      ~ONLCR;  // Prevent conversion of newline to carriage return/line feed

  tty.c_cc[VTIME] = 0;  // Wait forever
  tty.c_cc[VMIN] = 1;   // Wait for at least 1 character before returning

  cfsetispeed(&tty, baud_rate);
  cfsetospeed(&tty, baud_rate);

  // Save tty settings, also checking for error
  if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
    printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
  }

  char buff[1024];
  uint32_t idx = 0;

  while (1) {
    int rval = read(serial_port, &buff[idx++], 1);
    if (rval == 0) {
      printf(".");
      continue;
    } else if (rval < 0) {
      printf("read error %d\n", rval);
      break;
    }

    // No overflows please
    if (idx >= sizeof(buff)) {
      idx = 0;
      continue;
    }

    if (buff[idx - 1] == '\n') {
      // Buffer is too short (Should be at least 1 char + CRC(3) + CLRF(2))
      if (idx <= (VCRC_LEN + 3)) {
        idx = 0;
        continue;
      }

      // Step back from CRLF
      idx -= 2;

      if (message_is_valid(buff, idx)) {
        // Step back from CRC
        idx -= 3;
        printf("%.*s\n", idx, buff);
      } else {
        printf("Invalid mesage: %.*s\n", idx, buff);
      }
      idx = 0;
    }
  }

  close(serial_port);
}
