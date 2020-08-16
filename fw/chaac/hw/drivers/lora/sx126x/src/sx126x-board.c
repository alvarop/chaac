/*
  ______                              _
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: SX126x driver specific target board functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <assert.h>
#include "os/mynewt.h"
#include "os/os.h"
#include "hal/hal_spi.h"
#include "bsp/bsp.h"
#include "radio/radio.h"
#include "sx126x/sx126x.h"
#include "sx126x-board.h"

void SX126xIoInit( void )
{
    struct hal_spi_settings spi_settings;
    int rc;

#if MYNEWT_VAL(SX126X_HAS_ANT_SW)
    rc = hal_gpio_init_out(SX126X_RXTX, 0);
    assert(rc == 0);
#endif

    rc = hal_gpio_init_out(RADIO_NSS, 1);
    assert(rc == 0);

    hal_spi_disable(RADIO_SPI_IDX);

    spi_settings.data_order = HAL_SPI_MSB_FIRST;
    spi_settings.data_mode = HAL_SPI_MODE0;
    spi_settings.baudrate = MYNEWT_VAL(SX126X_SPI_BAUDRATE);
    spi_settings.word_size = HAL_SPI_WORD_SIZE_8BIT;

    rc = hal_spi_config(RADIO_SPI_IDX, &spi_settings);
    assert(rc == 0);

    rc = hal_spi_enable(RADIO_SPI_IDX);
    assert(rc == 0);

    rc = hal_gpio_init_in(SX126X_BUSY, HAL_GPIO_PULL_NONE);
    assert(rc == 0);

    rc = hal_gpio_init_in(SX126X_DIO1, HAL_GPIO_PULL_NONE);
    assert(rc == 0);
}

void SX126xIoIrqInit( DioIrqHandler dioIrq )
{
    if (dioIrq != NULL) {
        int rc = hal_gpio_irq_init(SX126X_DIO1, dioIrq, NULL,
                               HAL_GPIO_TRIG_RISING, HAL_GPIO_PULL_NONE);
        assert(rc == 0);
        hal_gpio_irq_enable(SX126X_DIO1);
    }

}

void SX126xIoDeInit( void )
{
    hal_gpio_irq_enable(SX126X_DIO1);
}

void SX126xReset( void )
{
    // Wait 6 ms
    os_cputime_delay_usecs(6000);
    
    // Set RESET pin to 0
    hal_gpio_init_out(SX126X_NRESET, 0);

    // Wait 20 ms
    os_cputime_delay_usecs(20000);

    // Configure RESET as input
    hal_gpio_init_in(SX126X_NRESET, HAL_GPIO_PULL_NONE);

    // Wait 10 ms
    os_cputime_delay_usecs(10000);
}

void SX126xWaitOnBusy( void )
{
    while( hal_gpio_read( SX126X_BUSY ) == 1 );
}

void SX126xWakeup( void )
{
    /*BoardDisableIrq( );*/

    hal_gpio_write(RADIO_NSS, 0);
    hal_spi_tx_val(RADIO_SPI_IDX, RADIO_GET_STATUS);
    hal_spi_tx_val(RADIO_SPI_IDX, 0x00);
    hal_gpio_write(RADIO_NSS, 1);

    // Wait for chip to be ready.
    SX126xWaitOnBusy( );

    /*BoardEnableIrq( );*/
}

void SX126xWriteCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
    SX126xCheckDeviceReady( );

    hal_gpio_write(RADIO_NSS, 0);

    hal_spi_tx_val(RADIO_SPI_IDX, (uint8_t) command);

    for( uint16_t i = 0; i < size; i++ )
    {
        hal_spi_tx_val(RADIO_SPI_IDX, (uint8_t) buffer[i]);
    }

    hal_gpio_write(RADIO_NSS, 1);

    if( command != RADIO_SET_SLEEP )
    {
        SX126xWaitOnBusy( );
    }
}

void SX126xReadCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
    SX126xCheckDeviceReady( );

    hal_gpio_write(RADIO_NSS, 0);

    hal_spi_tx_val(RADIO_SPI_IDX, (uint8_t) command);
    hal_spi_tx_val(RADIO_SPI_IDX, 0);

    for( uint16_t i = 0; i < size; i++ )
    {
        buffer[i] = hal_spi_tx_val(RADIO_SPI_IDX, 0);
    }

    hal_gpio_write(RADIO_NSS, 1);

    SX126xWaitOnBusy( );
}

void SX126xWriteRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
    SX126xCheckDeviceReady( );

    hal_gpio_write(RADIO_NSS, 0);

    hal_spi_tx_val(RADIO_SPI_IDX, RADIO_WRITE_REGISTER);
    hal_spi_tx_val(RADIO_SPI_IDX, ( address & 0xFF00 ) >> 8);
    hal_spi_tx_val(RADIO_SPI_IDX, address & 0x00FF);

    for( uint16_t i = 0; i < size; i++ )
    {
        hal_spi_tx_val(RADIO_SPI_IDX, (uint8_t) buffer[i]);
    }

    hal_gpio_write(RADIO_NSS, 1);

    SX126xWaitOnBusy( );
}

void SX126xWriteRegister( uint16_t address, uint8_t value )
{
    SX126xWriteRegisters( address, &value, 1 );
}

void SX126xReadRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
    SX126xCheckDeviceReady( );

    hal_gpio_write(RADIO_NSS, 0);

    hal_spi_tx_val(RADIO_SPI_IDX, RADIO_READ_REGISTER);
    hal_spi_tx_val(RADIO_SPI_IDX, ( address & 0xFF00 ) >> 8);
    hal_spi_tx_val(RADIO_SPI_IDX, address & 0x00FF);
    hal_spi_tx_val(RADIO_SPI_IDX, 0);

    for( uint16_t i = 0; i < size; i++ )
    {
        buffer[i] = hal_spi_tx_val(RADIO_SPI_IDX, 0);
    }

    hal_gpio_write(RADIO_NSS, 1);

    SX126xWaitOnBusy( );
}

uint8_t SX126xReadRegister( uint16_t address )
{
    uint8_t data;
    SX126xReadRegisters( address, &data, 1 );
    return data;
}

void SX126xWriteBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    SX126xCheckDeviceReady( );

    hal_gpio_write(RADIO_NSS, 0);

    hal_spi_tx_val(RADIO_SPI_IDX, RADIO_WRITE_BUFFER);
    hal_spi_tx_val(RADIO_SPI_IDX, offset);

    for( uint16_t i = 0; i < size; i++ )
    {
        hal_spi_tx_val(RADIO_SPI_IDX, (uint8_t) buffer[i]);
    }

    hal_gpio_write(RADIO_NSS, 1);

    SX126xWaitOnBusy( );
}

void SX126xReadBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    SX126xCheckDeviceReady( );

    hal_gpio_write(RADIO_NSS, 0);

    hal_spi_tx_val(RADIO_SPI_IDX, RADIO_READ_BUFFER);
    hal_spi_tx_val(RADIO_SPI_IDX, offset);
    hal_spi_tx_val(RADIO_SPI_IDX, 0);

    for( uint16_t i = 0; i < size; i++ )
    {
        buffer[i] = hal_spi_tx_val(RADIO_SPI_IDX, 0);
    }

    hal_gpio_write(RADIO_NSS, 1);

    SX126xWaitOnBusy( );
}

void SX126xSetRfTxPower( int8_t power )
{
    SX126xSetTxParams( power, RADIO_RAMP_40_US );
}

uint8_t SX126xGetPaSelect( uint32_t channel )
{
#if MYNEWT_VAL_CHOICE(SX126X_DEVICE_TYPE, SX1261)
    return SX1261;
#elif MYNEWT_VAL_CHOICE(SX126X_DEVICE_TYPE, SX1262)
    return SX1262;
#else
#error Must select device type!
#endif
}

void SX126xAntSwOn( void )
{
    if(MODE_TX == SX126xGetOperatingMode()) {
#if MYNEWT_VAL(SX126X_HAS_ANT_SW)
        hal_gpio_write(SX126X_RXTX, 1);
#elif MYNEWT_VAL(SX126X_HAS_COMP_ANT_SW)
        hal_gpio_write(SX126X_RXTX, 0);
        hal_gpio_write(SX126X_N_RXTX, 1);
#endif
    } else { // MODE_RX, MODE_RX_DC, MODE_CAD
#if MYNEWT_VAL(SX126X_HAS_ANT_SW)
        hal_gpio_write(SX126X_RXTX, 0);
#elif MYNEWT_VAL(SX126X_HAS_COMP_ANT_SW)
        hal_gpio_write(SX126X_RXTX, 1);
        hal_gpio_write(SX126X_N_RXTX, 0);
#endif

    }

}

void SX126xAntSwOff( void )
{
#if MYNEWT_VAL(SX126X_HAS_ANT_SW)
    hal_gpio_write(SX126X_RXTX, 0);
#elif MYNEWT_VAL(SX126X_HAS_COMP_ANT_SW)
    hal_gpio_write(SX126X_RXTX, 0);
    hal_gpio_write(SX126X_N_RXTX, 0);
#endif
}

bool SX126xCheckRfFrequency( uint32_t frequency )
{
    // Implement check. Currently all frequencies are supported
    return true;
}

