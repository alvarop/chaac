#ifndef __FIFO_H__
#define __FIFO_H__

#include <stdint.h>

typedef struct {
  uint32_t size_mask;
  uint8_t *buff;
  uint32_t start;
  uint32_t end;
} fifo_t;

int32_t fifo_init(fifo_t *fifo, uint32_t size, void *buff);
uint32_t fifo_size(fifo_t *fifo);
uint8_t fifo_push(fifo_t *fifo, uint8_t byte);
uint8_t fifo_pop(fifo_t *fifo);
uint8_t fifo_peek(fifo_t *fifo, uint32_t byte);

#endif
