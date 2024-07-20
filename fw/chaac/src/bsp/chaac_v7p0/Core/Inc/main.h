/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_gpio.h"
#include "stm32l4xx_ll_exti.h"
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_cortex.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_pwr.h"
#include "stm32l4xx_ll_dma.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void SystemClock_Config(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BATT_Pin LL_GPIO_PIN_0
#define BATT_GPIO_Port GPIOA
#define SOLAR_Pin LL_GPIO_PIN_1
#define SOLAR_GPIO_Port GPIOA
#define RADIO_CS_Pin LL_GPIO_PIN_4
#define RADIO_CS_GPIO_Port GPIOA
#define RS232_EN_Pin LL_GPIO_PIN_0
#define RS232_EN_GPIO_Port GPIOB
#define NINVALID_Pin LL_GPIO_PIN_1
#define NINVALID_GPIO_Port GPIOB
#define RADIO_DIO1_Pin LL_GPIO_PIN_8
#define RADIO_DIO1_GPIO_Port GPIOA
#define RADIO_DIO1_EXTI_IRQn EXTI9_5_IRQn
#define RADIO_DIO2_Pin LL_GPIO_PIN_9
#define RADIO_DIO2_GPIO_Port GPIOA
#define LED2_Pin LL_GPIO_PIN_15
#define LED2_GPIO_Port GPIOA
#define LED1_Pin LL_GPIO_PIN_3
#define LED1_GPIO_Port GPIOB
#define RADIO_RXEN_Pin LL_GPIO_PIN_4
#define RADIO_RXEN_GPIO_Port GPIOB
#define RADIO_TXEN_Pin LL_GPIO_PIN_5
#define RADIO_TXEN_GPIO_Port GPIOB
#define RADIO_NRST_Pin LL_GPIO_PIN_6
#define RADIO_NRST_GPIO_Port GPIOB
#define RADIO_BUSY_Pin LL_GPIO_PIN_7
#define RADIO_BUSY_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
