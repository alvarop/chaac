#include "IOAdc.h"
#include "adc.h"

#ifndef ADC_REF_MV
#define ADC_REF_MV 3000
#endif


BaseType_t xIOAdcInit(void *pvHandle) {
    configASSERT(pvHandle != 0);
    ADC_HandleTypeDef *pxHandle = (ADC_HandleTypeDef *)pvHandle;

    MX_ADC1_Init();

    // Calibrate ADC
    HAL_ADCEx_Calibration_Start(pxHandle, ADC_SINGLE_ENDED);
    return 0;
}

BaseType_t xIOAdcDeInit(void *pvHandle) {
    configASSERT(pvHandle != 0);
    ADC_HandleTypeDef *pxHandle = (ADC_HandleTypeDef *)pvHandle;

    HAL_ADC_DeInit(pxHandle);
    return 0;
}

AdcStatus_t xIOAdcRead(void *pvHandle, int32_t *plValue) {
    configASSERT(pvHandle != 0);
    ADC_HandleTypeDef *pxHandle = (ADC_HandleTypeDef *)pvHandle;

    HAL_ADC_Start(pxHandle);
    // TODO - use interrupts
    HAL_ADC_PollForConversion(pxHandle, 10);
    int32_t lResult = HAL_ADC_GetValue(pxHandle);
    HAL_ADC_Stop(pxHandle);

    if (plValue != NULL) {
        *plValue = lResult;
    }

    return 0;
}

AdcStatus_t xIOAdcReadMv(void *pvHandle, int32_t *plValueMv) {
    configASSERT(pvHandle != NULL);
    configASSERT(plValueMv != NULL);

    AdcStatus_t xRval = xIOAdcRead(pvHandle, plValueMv);
    if (xRval == ADC_OK) {
        *plValueMv = (*plValueMv * ADC_REF_MV)/(1 << 12);
    }

    return xRval;
}

AdcStatus_t xIOAdcConfig(void *pvHandle, void *pvConfig) {
    (void)pvHandle;
    (void)pvConfig;

    // Not implemented since adc config is taken care of by the handle and adcinit

    return 0;
}

AdcStatus_t xIOAdcChannelConfig(void *pvHandle, void *pvChannelConfig) {
    configASSERT(pvHandle != 0);
    ADC_HandleTypeDef *pxHandle = (ADC_HandleTypeDef *)pvHandle;
    ADC_ChannelConfTypeDef *pxConfig = (ADC_ChannelConfTypeDef *)pvChannelConfig;

    AdcStatus_t xRval = ADC_OK;

    if (HAL_ADC_ConfigChannel(pxHandle, pxConfig) != HAL_OK) {
        xRval = ADC_ERR;
    }

    return xRval;
}

