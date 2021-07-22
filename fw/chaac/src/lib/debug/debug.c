#include <stdint.h>
#include "main.h"

void vDebugInit(){

}

void _putchar(char cChar) {
  // Wait until ready to transmit
  while(!LL_USART_IsActiveFlag_TXE(USART2)){};

  LL_USART_TransmitData8(USART2, (uint8_t)cChar);
}


