#include "stm32l4xx_hal.h"
#include "main.h"

void app_main() {   
    while(1) {
        HAL_Delay(100);
        LL_GPIO_SetOutputPin(LED1_GPIO_Port, LED1_Pin);
        HAL_Delay(25);
        LL_GPIO_ResetOutputPin(LED1_GPIO_Port, LED1_Pin);
        HAL_Delay(100);
        LL_GPIO_SetOutputPin(RADIO_TXEN_GPIO_Port, RADIO_TXEN_Pin);
        HAL_Delay(25);
        LL_GPIO_ResetOutputPin(RADIO_TXEN_GPIO_Port, RADIO_TXEN_Pin);
    }
}

