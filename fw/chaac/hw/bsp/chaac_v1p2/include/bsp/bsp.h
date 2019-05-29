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
#include <mcu/mcu.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Define special stack sections */
#define sec_data_core   __attribute__((section(".data.core")))
#define sec_bss_core    __attribute__((section(".bss.core")))
#define sec_bss_nz_core __attribute__((section(".bss.core.nz")))

/* More convenient section placement macros. */
#define bssnz_t         sec_bss_nz_core

extern uint8_t _ram_start;

#define RAM_SIZE        (64 * 1024)

#define CHAAC_HW_VERS   (0x0102)

// XOR uid 32-bit words to have a single 32-bit uid
#define DEVICE_UID (   ((uint32_t *)(0x1FFF7590))[0] ^ \
                        ((uint32_t *)(0x1FFF7590))[1] ^ \
                        ((uint32_t *)(0x1FFF7590))[2])

#define XBEE_ON_PIN     MCU_GPIO_PORTA(0)
#define XBEE_nSBY_PIN   MCU_GPIO_PORTA(1)

#define XBEE_RX_PIN     MCU_GPIO_PORTA(2)
#define XBEE_TX_PIN     MCU_GPIO_PORTA(3)

#define XBEE_nRST_PIN   MCU_GPIO_PORTA(4)

#define BATT_PIN        MCU_GPIO_PORTA(5)
#define BATT_ADC_CH     (10)

#define LIGHT_PIN       MCU_GPIO_PORTA(6)
#define LIGHT_ADC_CH    (11)

#define WX_SPEED_PIN    MCU_GPIO_PORTA(7)
#define WX_RAIN_PIN     MCU_GPIO_PORTA(8)

#define SCL_PIN         MCU_GPIO_PORTA(9)
#define SDA_PIN         MCU_GPIO_PORTA(10)

#define LED1_PIN        MCU_GPIO_PORTA(12)
#define LED2_PIN        MCU_GPIO_PORTB(5)

#define BOOT_PIN        MCU_GPIO_PORTB(4)

#define WX_DIR_PIN      MCU_GPIO_PORTB(0)
#define WX_DIR_ADC_CH   (15)

#define VSOLAR_PIN      MCU_GPIO_PORTB(1)
#define VSOLAR_ADC_CH   (16)

#define WX_DIR_EN_PIN   MCU_GPIO_PORTB(3)
#define WX_DIR_EN_ON    (0)
#define WX_DIR_EN_OFF   (1)

#define UART_TX_PIN     MCU_GPIO_PORTB(6)
#define UART_RX_PIN     MCU_GPIO_PORTB(7)

#define LED_BLINK_PIN   LED1_PIN

/* UART */
#define UART_CNT 2

#ifdef __cplusplus
}
#endif

#endif  /* H_BSP_H */
