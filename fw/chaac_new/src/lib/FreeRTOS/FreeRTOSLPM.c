#include <stdbool.h>
#include "FreeRTOS.h"
#include "FreeRTOSLPM.h"
#include "stm32l4xx_hal.h"
#include "task.h"


// Simplified version of Jeff Tenney's ulp.c
// See: https://github.com/jefftenney/LPTIM-Tick/blob/main/Core/Src/ulp.c

static volatile bool lpmEnabled = false;

void lpmInit() {
   if ( (RCC->CFGR & RCC_CFGR_SWS_Msk) == RCC_CFGR_SWS_HSI ) {
      SET_BIT(RCC->CFGR, RCC_CFGR_STOPWUCK);
   } else if ( (RCC->CFGR & RCC_CFGR_SWS_Msk) == RCC_CFGR_SWS_MSI ) {
      CLEAR_BIT(RCC->CFGR, RCC_CFGR_STOPWUCK);
   }
}

void lpmEnable() {
    taskENTER_CRITICAL();
    lpmEnabled = true;
    taskEXIT_CRITICAL();
}

void lpmDisable() {
    taskENTER_CRITICAL();
    lpmEnabled = false;
    taskEXIT_CRITICAL();
}

void lpmEnableFromISR() {
    UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
    lpmEnabled = true;
    taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
}

void lpmDisableFromISR() {
    UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
    lpmEnabled = false;
    taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
}

static uint32_t rccCfgrSave;
static uint32_t rccCrSave;

void lpmPreSleepProcessing() {
    if (lpmEnabled) {
        MODIFY_REG(PWR->CR1, PWR_CR1_LPMS_Msk, PWR_CR1_LPMS_STOP2);

        rccCrSave = RCC->CR;
        rccCfgrSave = RCC->CFGR;

        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    }
}

void lpmPostSleepProcessing() {
    if (SCB->SCR & SCB_SCR_SLEEPDEEP_Msk) {
        //      We may have been in deep sleep.  If we were, the hardware cleared several enable bits in the CR,
        // and it changed the selected system clock in CFGR.  Restore them now.  If we're restarting the PLL as
        // the CPU clock here, the CPU will not wait for it.  Instead, the CPU continues executing from the
        // wake-up clock (MSI in our case) until the PLL is stable and then the CPU starts using the PLL.
        //
        RCC->CR = rccCrSave;
        RCC->CFGR = rccCfgrSave;

        SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;

        //      This application bypasses the RTC shadow registers, so we don't need to clear the sync flag for
        // those registers.  They are always out of sync when coming out of deep sleep.
        //
        // RTC->ISR &= ~RTC_ISR_RSF;
    }
}
