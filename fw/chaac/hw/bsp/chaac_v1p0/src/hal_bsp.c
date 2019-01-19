/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include <assert.h>

#include "os/mynewt.h"

#if MYNEWT_VAL(UART_0) || MYNEWT_VAL(UART_1)
#include <uart/uart.h>
#include <uart_hal/uart_hal.h>
#endif

#include <hal/hal_bsp.h>
#include <hal/hal_gpio.h>
#include <hal/hal_flash_int.h>
#include <hal/hal_timer.h>

#if MYNEWT_VAL(SPI_0_MASTER) || MYNEWT_VAL(SPI_0_SLAVE)
#include <hal/hal_spi.h>
#endif

#include <stm32l432xx.h>
#include <stm32l4xx_hal_dma.h>
#include <stm32l4xx_hal_adc.h>
#include <stm32l4xx_hal_rcc.h>
#include <stm32l4xx_hal_pwr.h>
#include <stm32l4xx_hal_flash.h>
#include <stm32l4xx_hal_gpio_ex.h>
#include <mcu/stm32l4_bsp.h>
#include "mcu/stm32l4xx_mynewt_hal.h"
#include "mcu/stm32_hal.h"
#include "hal/hal_i2c.h"

#if MYNEWT_VAL(ADC_1)
#include <adc_stm32l432/adc_stm32l432.h>
#endif

#include "bsp/bsp.h"

#if MYNEWT_VAL(UART_0)
static struct uart_dev hal_uart0;
static struct uart_dev hal_uart1;

static const struct stm32_uart_cfg uart_cfg[UART_CNT] = {
    [0] = {
        .suc_uart = USART2,
        .suc_rcc_reg = &RCC->APB1ENR1,
        .suc_rcc_dev = RCC_APB1ENR1_USART2EN,
        .suc_pin_tx = MCU_GPIO_PORTA(2),
        .suc_pin_rx = MCU_GPIO_PORTA(3),
        .suc_pin_rts = -1,
        .suc_pin_cts = -1,
        .suc_pin_af = GPIO_AF7_USART2,
        // TODO - tx and rx have different af mapping
        // tx is GPIO_AF3_USART2 and rx is GPIO_AF7_USART2
        .suc_irqn = USART2_IRQn
    },
    [1] = {
        .suc_uart = USART1,
        .suc_rcc_reg = &RCC->APB2ENR,
        .suc_rcc_dev = RCC_APB2ENR_USART1EN,
        .suc_pin_tx = MCU_GPIO_PORTB(6),
        .suc_pin_rx = MCU_GPIO_PORTB(7),
        .suc_pin_rts = -1,
        .suc_pin_cts = -1,
        .suc_pin_af = GPIO_AF7_USART1,
        .suc_irqn = USART1_IRQn
    }
};
#endif

#if MYNEWT_VAL(I2C_0)

static struct stm32_hal_i2c_cfg i2c_cfg0 = {
    .hic_i2c = I2C1,
    .hic_rcc_reg = &RCC->APB1ENR1,
    .hic_rcc_dev = RCC_APB1ENR1_I2C1EN,
    .hic_pin_sda = MCU_GPIO_PORTA(10),
    .hic_pin_scl = MCU_GPIO_PORTA(9),
    .hic_pin_af = GPIO_AF4_I2C1,
    .hic_10bit = 0,
    .hic_timingr = 0x00303D5B,            /* 100KHz at 16MHz of SysCoreClock */
};
#endif

#if MYNEWT_VAL(SPI_0_SLAVE) || MYNEWT_VAL(SPI_0_MASTER)
struct stm32_hal_spi_cfg spi0_cfg = {
    .ss_pin   = MCU_GPIO_PORTA(4),
    .sck_pin  = MCU_GPIO_PORTA(5),
    .miso_pin = MCU_GPIO_PORTA(6),
    .mosi_pin = MCU_GPIO_PORTB(5),
    .irq_prio = 2,
};
#endif

static const struct hal_bsp_mem_dump dump_cfg[] = {
    [0] = {
        .hbmd_start = &_ram_start,
        .hbmd_size = RAM_SIZE
    },
};

#if MYNEWT_VAL(ADC_1)
struct adc_dev my_dev_adc1;


#define STM32L432_ADC_DEFAULT_INIT_TD {\
    .ClockPrescaler = ADC_CLOCK_ASYNC_DIV1,\
    .Resolution = ADC_RESOLUTION_12B,\
    .DataAlign = ADC_DATAALIGN_RIGHT,\
    .ScanConvMode = ADC_SCAN_DISABLE,\
    .EOCSelection = ADC_EOC_SINGLE_CONV,\
    .LowPowerAutoWait = DISABLE,\
    .ContinuousConvMode = DISABLE,\
    .NbrOfConversion = 1,\
    .DiscontinuousConvMode = DISABLE,\
    .ExternalTrigConv = ADC_SOFTWARE_START,\
    .ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE,\
    .DMAContinuousRequests = DISABLE,\
    .Overrun = ADC_OVR_DATA_PRESERVED,\
    .OversamplingMode = ENABLE,\
    .Oversampling = {\
      .Ratio = ADC_OVERSAMPLING_RATIO_256, \
      .RightBitShift = ADC_RIGHTBITSHIFT_8, \
      .TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER, \
      .OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE, \
    },\
}

#define STM32L432_DEFAULT_ADC1_HANDLE {\
    .Init = STM32L432_ADC_DEFAULT_INIT_TD,\
    .Instance = ADC1,\
    .DMA_Handle = NULL,\
    .Lock = HAL_UNLOCKED,\
    .State = 0,\
    .ErrorCode = 0,\
    .InjectionConfig={0,0}\
}

ADC_HandleTypeDef adc1_handle = STM32L432_DEFAULT_ADC1_HANDLE;

// VBATT
#define ADC_CH10_SAC_CFG {\
    .c_refmv = 3300,\
    .c_res   = 12,\
    .c_configured = 1,\
    .c_cnum = 10\
}

// LIGHT
#define ADC_CH11_SAC_CFG {\
    .c_refmv = 3300,\
    .c_res   = 12,\
    .c_configured = 1,\
    .c_cnum = 11\
}

//WDIR
#define ADC_CH15_SAC_CFG {\
    .c_refmv = 3300,\
    .c_res   = 12,\
    .c_configured = 1,\
    .c_cnum = 15\
}

#define STM32L432_ADC1_DEFAULT_CONFIG {\
    .sac_chan_count = 16,\
    .sac_chans = (struct adc_chan_config [16]){\
        {0},\
        {0},\
        {0},\
        {0},\
        {0},\
        {0},\
        {0},\
        {0},\
        {0},\
        {0},\
        ADC_CH10_SAC_CFG,  /* BATT pin (PA5) */   \
        ADC_CH11_SAC_CFG,  /* LIGHT pin (PA6) */  \
        {0},\
        {0},\
        {0},\
        ADC_CH15_SAC_CFG   /* WDIR pin (PB0) */   \
      },\
    .sac_adc_handle = &adc1_handle,\
  }

struct stm32l432_adc_dev_cfg adc1_config = STM32L432_ADC1_DEFAULT_CONFIG;

#endif

extern const struct hal_flash stm32_flash_dev;
const struct hal_flash *
hal_bsp_flash_dev(uint8_t id)
{
    /*
     * Internal flash mapped to id 0.
     */
    if (id != 0) {
        return NULL;
    }
    return &stm32_flash_dev;
}

const struct hal_bsp_mem_dump *
hal_bsp_core_dump(int *area_cnt)
{
    *area_cnt = sizeof(dump_cfg) / sizeof(dump_cfg[0]);
    return dump_cfg;
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /**Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /**Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_8;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  assert(HAL_RCC_OscConfig(&RCC_OscInitStruct) == HAL_OK);

  /**Initializes the CPU, AHB and APB busses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  assert(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) == HAL_OK);

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_ADC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
  PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_MSI;
  PeriphClkInit.PLLSAI1.PLLSAI1M = 1;
  PeriphClkInit.PLLSAI1.PLLSAI1N = 8;
  PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV7;
  PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_ADC1CLK;
  assert(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) == HAL_OK);

  /**Configure the main internal regulator output voltage
  */
  assert(HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) == HAL_OK);

  /**Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

void
hal_bsp_init(void)
{
    int rc;

    (void)rc;

    SystemClock_Config();
    SystemCoreClockUpdate();

#if MYNEWT_VAL(UART_0)
    rc = os_dev_create((struct os_dev *) &hal_uart0, "uart0",
      OS_DEV_INIT_PRIMARY, 0, uart_hal_init, (void *)&uart_cfg[0]);
    assert(rc == 0);
#endif

#if MYNEWT_VAL(UART_1)
    rc = os_dev_create((struct os_dev *) &hal_uart1, "uart1",
      OS_DEV_INIT_PRIMARY, 0, uart_hal_init, (void *)&uart_cfg[1]);
    assert(rc == 0);
#endif

#if MYNEWT_VAL(ADC_1)
    rc = os_dev_create((struct os_dev *) &my_dev_adc1, "adc1",
            OS_DEV_INIT_KERNEL, OS_DEV_INIT_PRIO_DEFAULT,
            stm32l432_adc_dev_init, &adc1_config);
    assert(rc == 0);
#endif

#if MYNEWT_VAL(TIMER_0)
    hal_timer_init(0, TIM15);
#endif

#if MYNEWT_VAL(TIMER_1)
    hal_timer_init(1, TIM16);
#endif

#if MYNEWT_VAL(TIMER_2)
    hal_timer_init(2, TIM2);
#endif


#if MYNEWT_VAL(OS_CPUTIME_TIMER_NUM) > 0
    rc = os_cputime_init(MYNEWT_VAL(OS_CPUTIME_FREQ));
    assert(rc == 0);
#endif

#if MYNEWT_VAL(SPI_0_MASTER)
    rc = hal_spi_init(0, &spi0_cfg, HAL_SPI_TYPE_MASTER);
    assert(rc == 0);
#endif

#if MYNEWT_VAL(SPI_0_SLAVE)
    rc = hal_spi_init(0, &spi0_cfg, HAL_SPI_TYPE_SLAVE);
    assert(rc == 0);
#endif

#if MYNEWT_VAL(I2C_0)
    rc = hal_i2c_init(0, &i2c_cfg0);
    assert(rc == 0);
#endif
}

/**
 * Returns the configured priority for the given interrupt. If no priority
 * configured, return the priority passed in
 *
 * @param irq_num
 * @param pri
 *
 * @return uint32_t
 */
uint32_t
hal_bsp_get_nvic_priority(int irq_num, uint32_t pri)
{
    /* Add any interrupt priorities configured by the bsp here */
    return pri;
}

void NMI_Handler() {while(1);}
void HardFault_Handler() {while(1);}
void MemManage_Handler() {while(1);}
void BusFault_Handler() {while(1);}
void UsageFault_Handler() {while(1);}
void WWDG_IRQHandler () { while(1);};
void PVD_PVM_IRQHandler () { while(1);};
void TAMP_STAMP_IRQHandler () { while(1);};
void RTC_WKUP_IRQHandler () { while(1);};
void FLASH_IRQHandler () { while(1);};
void RCC_IRQHandler () { while(1);};
void EXTI0_IRQHandler () { while(1);};
void EXTI1_IRQHandler () { while(1);};
void EXTI2_IRQHandler () { while(1);};
void EXTI3_IRQHandler () { while(1);};
void EXTI4_IRQHandler () { while(1);};
void DMA1_Channel1_IRQHandler () { while(1);};
void DMA1_Channel2_IRQHandler () { while(1);};
void DMA1_Channel3_IRQHandler () { while(1);};
void DMA1_Channel4_IRQHandler () { while(1);};
void DMA1_Channel5_IRQHandler () { while(1);};
void DMA1_Channel6_IRQHandler () { while(1);};
void DMA1_Channel7_IRQHandler () { while(1);};
void ADC1_IRQHandler () { while(1);};
void CAN1_TX_IRQHandler () { while(1);};
void CAN1_RX0_IRQHandler () { while(1);};
void CAN1_RX1_IRQHandler () { while(1);};
void CAN1_SCE_IRQHandler () { while(1);};
void EXTI9_5_IRQHandler () { while(1);};
void TIM1_BRK_TIM15_IRQHandler () { while(1);};
void TIM1_UP_TIM16_IRQHandler () { while(1);};
void TIM1_TRG_COM_IRQHandler () { while(1);};
void TIM1_CC_IRQHandler () { while(1);};
void TIM2_IRQHandler () { while(1);};
void I2C1_EV_IRQHandler () { while(1);};
void I2C1_ER_IRQHandler () { while(1);};
void SPI1_IRQHandler () { while(1);};
void USART1_IRQHandler () { while(1);};
void USART2_IRQHandler () { while(1);};
void EXTI15_10_IRQHandler () { while(1);};
void RTC_Alarm_IRQHandler () { while(1);};
void SPI3_IRQHandler () { while(1);};
void TIM6_DAC_IRQHandler () { while(1);};
void TIM7_IRQHandler () { while(1);};
void DMA2_Channel1_IRQHandler () { while(1);};
void DMA2_Channel2_IRQHandler () { while(1);};
void DMA2_Channel3_IRQHandler () { while(1);};
void DMA2_Channel4_IRQHandler () { while(1);};
void DMA2_Channel5_IRQHandler () { while(1);};
void COMP_IRQHandler () { while(1);};
void LPTIM1_IRQHandler () { while(1);};
void LPTIM2_IRQHandler () { while(1);};
void USB_IRQHandler () { while(1);};
void DMA2_Channel6_IRQHandler () { while(1);};
void DMA2_Channel7_IRQHandler () { while(1);};
void LPUART1_IRQHandler () { while(1);};
void QUADSPI_IRQHandler () { while(1);};
void I2C3_EV_IRQHandler () { while(1);};
void I2C3_ER_IRQHandler () { while(1);};
void SAI1_IRQHandler () { while(1);};
void SWPMI1_IRQHandler () { while(1);};
void TSC_IRQHandler () { while(1);};
void RNG_IRQHandler () { while(1);};
void FPU_IRQHandler () { while(1);};
void CRS_IRQHandler () { while(1);};
