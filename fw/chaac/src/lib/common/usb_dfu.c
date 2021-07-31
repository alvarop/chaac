#include <stdint.h>
#include "stm32l4xx_hal.h"

#define DFU_MAGIC 0x9FBFA328
#define BOOTLOADER_ADDR 0x1FFF0000

typedef void (*functionPointer_t)(void);

uint32_t dfuMagic __attribute__ ((section (".noinit")));

void dfuReset() {
  // Set magic value to enter bootloader after reset
  dfuMagic = DFU_MAGIC;

  NVIC_SystemReset();
}

void dfuCheck() {
  uint32_t cachedDFUMagic = dfuMagic;

  dfuMagic = 0;

  if(cachedDFUMagic == DFU_MAGIC) {
    functionPointer_t bootloader = (functionPointer_t)(*(volatile uint32_t *)(BOOTLOADER_ADDR + 4));

    // Set new SP
    __set_MSP(*(volatile uint32_t *)BOOTLOADER_ADDR);

    // Enter bootloader
    bootloader();
  }
}
