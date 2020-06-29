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

#ifndef H_BSP_H
#define H_BSP_H

#include <inttypes.h>

#include <syscfg/syscfg.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Define special stackos sections */
#define sec_data_core   __attribute__((section(".data.core")))
#define sec_bss_core    __attribute__((section(".bss.core")))
#define sec_bss_nz_core __attribute__((section(".bss.core.nz")))

/* More convenient section placement macros. */
#define bssnz_t         sec_bss_nz_core

extern uint8_t _ram_start;
#define RAM_SIZE        0x10000

#define CHAAC_HW_VERS   (0x0400)

// XOR uid 32-bit words to have a single 32-bit uid
#define DEVICE_UID      (NRF_FICR->DEVICEID[0] ^ NRF_FICR->DEVICEID[1])

#define BATT_PIN        28
#define BATT_ADC_CH     4
#define BATT_SAADC      NRF_SAADC_INPUT_AIN4

#define WX_SPEED_PIN    29
#define WX_RAIN_PIN     31

#define LED1_PIN        6
#define LED2_PIN        5

#define BOOT_PIN        22

#define WX_DIR_PIN      30
#define WX_DIR_ADC_CH   6
#define WX_DIR_SAADC    NRF_SAADC_INPUT_AIN6

// TODO - remove vsolar
#define VSOLAR_PIN      4
#define VSOLAR_ADC_CH   2
#define VSOLAR_SAADC    NRF_SAADC_INPUT_AIN2

#define WX_DIR_EN_PIN   25
#define WX_DIR_EN_ON    (0)
#define WX_DIR_EN_OFF   (1)

// LORA radio
#define SX126X_DIO1     11
#define SX126X_BUSY     12
#define SX126X_NRESET   13
#define SX126X_DIO2     20
#define E22_RXEN        18
#define E22_TXEN        19

#define LED_BLINK_PIN   LED1_PIN

/* UART */
#define UART_CNT 1

#ifdef __cplusplus
}
#endif

#endif  /* H_BSP_H */
