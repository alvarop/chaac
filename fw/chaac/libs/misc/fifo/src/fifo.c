#include <fifo/fifo.h>
#include <os/os.h>

// Set up fifo struct (Buffer size must be a power of 2)
int32_t fifo_init(fifo_t *fifo, uint32_t size, void *buff) {
  if (!(size && !(size & (size - 1)))) {
    // ERROR - Fifo size not a power of 2
    return -1;
  }

  fifo->size_mask = size - 1;
  fifo->buff = (uint8_t *)buff;
  fifo->start = 0;
  fifo->end = 0;

  return 0;
}

uint32_t fifo_size(fifo_t *fifo) {
  return (fifo->end - fifo->start) & fifo->size_mask;
}

uint8_t fifo_push(fifo_t *fifo, uint8_t byte) {
  uint8_t rval = 0;
  int sr;

  OS_ENTER_CRITICAL(sr);

  fifo->buff[fifo->end++] = byte;
  fifo->end &= fifo->size_mask;

  // If start==end, we've looped around and will start dropping characters
  if (fifo->start == fifo->end) {
    fifo->start = (fifo->start + 1) & fifo->size_mask;
    rval = 1;
  }

  OS_EXIT_CRITICAL(sr);

  return rval;
}

uint8_t fifo_pop(fifo_t *fifo) {
  uint8_t byte = 0;
  int sr;

  OS_ENTER_CRITICAL(sr);

  // Make sure the fifo is not empty
  if (fifo->start != fifo->end) {
    byte = fifo->buff[fifo->start++];
    fifo->start &= fifo->size_mask;
  }

  OS_EXIT_CRITICAL(sr);

  return byte;
}

uint8_t fifo_peek(fifo_t *fifo, uint32_t byte) {
  return fifo->buff[(fifo->start + byte) & fifo->size_mask];
}
