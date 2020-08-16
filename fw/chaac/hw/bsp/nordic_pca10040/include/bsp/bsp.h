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

#include "os/mynewt.h"

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

#define CHAAC_HW_VERS   (0x0ff0)

// XOR uid 32-bit words to have a single 32-bit uid
#define DEVICE_UID      (NRF_FICR->DEVICEID[0] ^ NRF_FICR->DEVICEID[1])


/* LED pins */
#define LED_1           (17)
#define LED_2           (18)
#define LED_3           (19)
#define LED_4           (20)
#define LED_BLINK_PIN   (LED_1)

/* Buttons */
#define BUTTON_1 (13)
#define BUTTON_2 (14)
#define BUTTON_3 (15)
#define BUTTON_4 (16)

/* Arduino pins */
#define ARDUINO_PIN_D0      11
#define ARDUINO_PIN_D1      12
#define ARDUINO_PIN_D2      13
#define ARDUINO_PIN_D3      14
#define ARDUINO_PIN_D4      15
#define ARDUINO_PIN_D5      16
#define ARDUINO_PIN_D6      17
#define ARDUINO_PIN_D7      18
#define ARDUINO_PIN_D8      19
#define ARDUINO_PIN_D9      20
#define ARDUINO_PIN_D10     22
#define ARDUINO_PIN_D11     23
#define ARDUINO_PIN_D12     24
#define ARDUINO_PIN_D13     25
#define ARDUINO_PIN_A0      3
#define ARDUINO_PIN_A1      4
#define ARDUINO_PIN_A2      28
#define ARDUINO_PIN_A3      29
#define ARDUINO_PIN_A4      30
#define ARDUINO_PIN_A5      31

#define ARDUINO_PIN_RX      ARDUINO_PIN_D0
#define ARDUINO_PIN_TX      ARDUINO_PIN_D1

#define ARDUINO_PIN_SCL     27
#define ARDUINO_PIN_SDA     26

#define ARDUINO_PIN_SCK     ARDUINO_PIN_D13
#define ARDUINO_PIN_MOSI    ARDUINO_PIN_D11
#define ARDUINO_PIN_MISO    ARDUINO_PIN_D12

#define BATT_PIN        28
#define BATT_ADC_CH     4
#define BATT_SAADC      NRF_SAADC_INPUT_AIN4

#define WX_SPEED_PIN    29
#define WX_RAIN_PIN     31

#define LED1_PIN        17
#define LED2_PIN        18

#define BOOT_PIN        22

#define WX_DIR_PIN      30
#define WX_DIR_ADC_CH   6
#define WX_DIR_SAADC    NRF_SAADC_INPUT_AIN6

#define WX_DIR_EN_PIN   14
#define WX_DIR_EN_ON    (0)
#define WX_DIR_EN_OFF   (1)

// LORA radio
/*#define SX126X_DIO1     13
#define SX126X_BUSY     16
#define SX126X_NRESET   8
#define SX126X_DIO2     7
#define E22_RXEN        6
#define E22_TXEN        5
*/

#define TX_LED_PIN LED_1
#define RX_LED_PIN LED_2
#define STATUS_LED_PIN LED_3


#ifdef __cplusplus
}
#endif

#endif  /* H_BSP_H */
