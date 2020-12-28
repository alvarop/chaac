#include <stdint.h>
#include "stm32l4xx.h"

#define TPIU_CURRENT_PORT_SIZE *((volatile unsigned *)(0xE0040004))
#define TPIU_ASYNC_CLOCK_PRESCALER *((volatile unsigned *)(0xE0040010))
#define TPIU_SELECTED_PIN_PROTOCOL *((volatile unsigned *)(0xE00400F0))
#define TPIU_FORMATTER_AND_FLUSH_CONTROL *((volatile unsigned *)(0xE0040304))

// TODO - do this from the debugger itself
static void prvSWOInit(uint32_t ulBaudRate) {
  uint32_t SWOPrescaler = (SystemCoreClock / ulBaudRate) - 1; /* SWOSpeed in Hz, note that cpuCoreFreqHz is expected to be match the CPU core clock */

  CoreDebug->DEMCR = CoreDebug_DEMCR_TRCENA_Msk;
  DBGMCU->CR = DBGMCU_CR_DBG_SLEEP_Msk | DBGMCU_CR_DBG_STOP_Msk | DBGMCU_CR_DBG_STANDBY_Msk | DBGMCU_CR_TRACE_IOEN_Msk;

  TPIU_CURRENT_PORT_SIZE = 1; /* port size = 1 bit */
  TPIU_SELECTED_PIN_PROTOCOL = 1; /* trace port protocol = Manchester */
  TPIU_ASYNC_CLOCK_PRESCALER = SWOPrescaler;
  TPIU_FORMATTER_AND_FLUSH_CONTROL = 0x100; /* turn off formatter (0x02 bit) */

  ITM->LAR = 0xC5ACCE55;
  ITM->TCR = ITM_TCR_TraceBusID_Msk | ITM_TCR_SWOENA_Msk | ITM_TCR_SYNCENA_Msk | ITM_TCR_ITMENA_Msk;
  ITM->TPR = ITM_TPR_PRIVMASK_Msk; /* all ports accessible unprivileged */
  ITM->TER = 1; /* enable stimulus channel 0, used with ITM_SendChar() */

  /* this apparently turns off sync packets, see SYNCTAP in DDI0403D pdf: */
  DWT->CTRL = 0x400003FE;
}

void vDebugInit(){
    prvSWOInit(32000);
}

void _putchar(char cChar) {
  ITM_SendChar(cChar);
}


