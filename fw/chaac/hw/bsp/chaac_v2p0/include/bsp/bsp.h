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

#define CHAAC_HW_VERS   (0x0102)

// XOR uid 32-bit words to have a single 32-bit uid
#define DEVICE_UID      (NRF_FICR->DEVICEID[0] ^ NRF_FICR->DEVICEID[1])

#define XBEE_ON_PIN     25
#define XBEE_nSBY_PIN   26

#define XBEE_RX_PIN     27
#define XBEE_TX_PIN     28

// UNUSED
#define XBEE_nRST_PIN   20

#define BATT_PIN        30
#define BATT_ADC_CH     6

// UNUSED
#define LIGHT_PIN       20
#define LIGHT_ADC_CH    0

#define WX_SPEED_PIN    4
#define WX_RAIN_PIN     7

#define SCL_PIN         9
#define SDA_PIN         8

#define LED1_PIN        11
#define LED2_PIN        12

#define BOOT_PIN        10

#define WX_DIR_PIN      5
#define WX_DIR_ADC_CH   3

#define VSOLAR_PIN      31
#define VSOLAR_ADC_CH   7

#define WX_DIR_EN_PIN   29
#define WX_DIR_EN_ON    (0)
#define WX_DIR_EN_OFF   (1)

#define UART_TX_PIN     13
#define UART_RX_PIN     14

#define LED_BLINK_PIN   LED1_PIN

/* UART */
#define UART_CNT 2

#ifdef __cplusplus
}
#endif

#endif  /* H_BSP_H */
