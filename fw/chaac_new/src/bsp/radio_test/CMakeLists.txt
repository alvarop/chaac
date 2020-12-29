set(BSP_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

set(BSP_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/stm32l4xx_hal_msp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/stm32l4xx_hal_timebase_tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/stm32l4xx_it.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/system_stm32l4xx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/usart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cortex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_exti.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ramfunc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_exti.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_rcc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_usart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/startup_stm32l476xx.s
    ${CMAKE_CURRENT_SOURCE_DIR}/sx1262-board.c
    PARENT_SCOPE)


set(BSP_INCLUDES

    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32L4xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Inc/Legacy

    PARENT_SCOPE)

set(BSP_DEFINES
    USE_FULL_LL_DRIVER
    USER_HAL_DRIVER
    STM32L476xx
    PARENT_SCOPE)
