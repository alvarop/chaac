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

#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include "os/mynewt.h"
#include "nrfx.h"
#include "flash_map/flash_map.h"
#include "hal/hal_bsp.h"
#include "hal/hal_flash.h"
#include "hal/hal_system.h"
#include "mcu/nrf52_hal.h"
#include "mcu/nrf52_periph.h"
#include "bsp/bsp.h"
#include "defs/sections.h"
#if MYNEWT_VAL(ENC_FLASH_DEV)
#include <ef_nrf5x/ef_nrf5x.h>
#endif

/*
 * What memory to include in coredump.
 */
static const struct hal_bsp_mem_dump dump_cfg[] = {
    [0] = {
        .hbmd_start = &_ram_start,
        .hbmd_size = RAM_SIZE
    }
};

#if MYNEWT_VAL(ENC_FLASH_DEV)
static sec_data_secret struct eflash_nrf5x_dev enc_flash_dev0 = {
    .end_dev = {
        .efd_hal = {
            .hf_itf = &enc_flash_funcs,
        },
        .efd_hwdev = &nrf52k_flash_dev
    }
};

#endif

const struct hal_flash *
hal_bsp_flash_dev(uint8_t id)
{
    /*
     * Internal flash mapped to id 0.
     */
    if (id == 0) {
        return &nrf52k_flash_dev;
    }
#if MYNEWT_VAL(ENC_FLASH_DEV)
    if (id == 1) {
        return &enc_flash_dev0.end_dev.efd_hal;
    }
#endif
    return NULL;
}

const struct hal_bsp_mem_dump *
hal_bsp_core_dump(int *area_cnt)
{
    *area_cnt = sizeof(dump_cfg) / sizeof(dump_cfg[0]);
    return dump_cfg;
}

int
hal_bsp_power_state(int state)
{
    return (0);
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
    uint32_t cfg_pri;

    switch (irq_num) {
    /* Radio gets highest priority */
    case RADIO_IRQn:
        cfg_pri = 0;
        break;
    default:
        cfg_pri = pri;
    }
    return cfg_pri;
}

void
hal_bsp_init(void)
{
    /* Make sure system clocks have started */
    hal_system_clock_start();

    /* Create all available nRF52840 peripherals */
    nrf52_periph_create();
}

#if MYNEWT_VAL(LORA_NODE)
void lora_bsp_enable_mac_timer(void)
{
    /* Turn on the LoRa MAC timer. This function is automatically
     * called by the LoRa stack when exiting low power mode.*/
    hal_timer_init(MYNEWT_VAL(LORA_MAC_TIMER_NUM), NULL);
}
#endif

void POWER_CLOCK_IRQHandler(){while(1);}
void RADIO_IRQHandler(){while(1);}
void UARTE0_UART0_IRQHandler(){while(1);}
void SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler(){while(1);}
void SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler(){while(1);}
void NFCT_IRQHandler(){while(1);}
void GPIOTE_IRQHandler(){while(1);}
void SAADC_IRQHandler(){while(1);}
void TIMER0_IRQHandler(){while(1);}
void TIMER1_IRQHandler(){while(1);}
void TIMER2_IRQHandler(){while(1);}
void RTC0_IRQHandler(){while(1);}
void TEMP_IRQHandler(){while(1);}
void RNG_IRQHandler(){while(1);}
void ECB_IRQHandler(){while(1);}
void CCM_AAR_IRQHandler(){while(1);}
void WDT_IRQHandler(){while(1);}
void RTC1_IRQHandler(){while(1);}
void QDEC_IRQHandler(){while(1);}
void COMP_LPCOMP_IRQHandler(){while(1);}
void SWI0_EGU0_IRQHandler(){while(1);}
void SWI1_EGU1_IRQHandler(){while(1);}
void SWI2_EGU2_IRQHandler(){while(1);}
void SWI3_EGU3_IRQHandler(){while(1);}
void SWI4_EGU4_IRQHandler(){while(1);}
void SWI5_EGU5_IRQHandler(){while(1);}
void TIMER3_IRQHandler(){while(1);}
void TIMER4_IRQHandler(){while(1);}
void PWM0_IRQHandler(){while(1);}
void PDM_IRQHandler(){while(1);}
void MWU_IRQHandler(){while(1);}
void PWM1_IRQHandler(){while(1);}
void PWM2_IRQHandler(){while(1);}
void SPIM2_SPIS2_SPI2_IRQHandler(){while(1);}
void RTC2_IRQHandler(){while(1);}
void I2S_IRQHandler(){while(1);}
