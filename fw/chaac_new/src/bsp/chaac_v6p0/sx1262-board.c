#include <stdlib.h>
// #include "utilities.h"
#include "radio.h"
#include "sx126x-board.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

/*!
 * \brief Holds the internal operating mode of the radio
 */
static RadioOperatingModes_t OperatingMode;

/*!
 * Antenna switch GPIO pins objects
 */
//Gpio_t AntPow;
//Gpio_t DeviceSel;

extern SPI_HandleTypeDef hspi1;

DioIrqHandler *SX126xdioIrq = NULL;

void SX126xIoInit( void )
{
    // GPIO initialized on bsp main
    /*GpioInit( &SX126x.Spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );*/
    /*GpioInit( &SX126x.BUSY, RADIO_BUSY, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );*/
    /*GpioInit( &SX126x.DIO1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );*/
    /*GpioInit( &DeviceSel, RADIO_DEVICE_SEL, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );*/

    // make sure CS is high
    LL_GPIO_SetOutputPin(RADIO_CS_GPIO_Port, RADIO_CS_Pin);
}

void SX126xIoIrqInit( DioIrqHandler dioIrq )
{
    (void)dioIrq;
    SX126xdioIrq = dioIrq;
    /*GpioSetInterrupt( &SX126x.DIO1, IRQ_RISING_EDGE, IRQ_HIGH_PRIORITY, dioIrq );*/
}

void SX126xIoDeInit( void )
{
    /*GpioInit( &SX126x.Spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );*/
    /*GpioInit( &SX126x.BUSY, RADIO_BUSY, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );*/
    /*GpioInit( &SX126x.DIO1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );*/
}

void SX126xIoDbgInit( void )
{

}

void SX126xIoTcxoInit( void )
{
    // Specific for E22 radio
    SX126xSetDio3AsTcxoCtrl( TCXO_CTRL_2_4V, 5000 );

    CalibrationParams_t calibParam;
    // Calibrate all blocks
    calibParam.Value = 0x7F;
    SX126xCalibrate( calibParam );
}

uint32_t SX126xGetBoardTcxoWakeupTime( void )
{
    // TODO - figure out this time with dio3 thing
    // return BOARD_TCXO_WAKEUP_TIME;
    return 20; // MADE UP NUMBER
}

void SX126xIoRfSwitchInit( void )
{
    /*SX126xSetDio2AsRfSwitchCtrl( true );*/
    LL_GPIO_ResetOutputPin(RADIO_TXEN_GPIO_Port, RADIO_TXEN_Pin);
    LL_GPIO_SetOutputPin(RADIO_RXEN_GPIO_Port, RADIO_RXEN_Pin);
}

RadioOperatingModes_t SX126xGetOperatingMode( void )
{
    return OperatingMode;
}

void SX126xSetOperatingMode( RadioOperatingModes_t mode )
{
    OperatingMode = mode;
}

void SX126xReset( void )
{
    vTaskDelay(pdMS_TO_TICKS( 10 ));

    LL_GPIO_ResetOutputPin(RADIO_NRST_GPIO_Port, RADIO_NRST_Pin);
    vTaskDelay(pdMS_TO_TICKS( 20 ));
    // TODO - internal pull up instead of pushpull like example?
    LL_GPIO_SetOutputPin(RADIO_NRST_GPIO_Port, RADIO_NRST_Pin);
    vTaskDelay(pdMS_TO_TICKS( 10 ));
}

void SX126xWaitOnBusy( void )
{
    while( LL_GPIO_IsInputPinSet(RADIO_BUSY_GPIO_Port, RADIO_BUSY_Pin) == 1 );
}

void SX126xWakeup( void )
{
    uint8_t pucTxBuff[] = {RADIO_GET_STATUS, 0x00};

    // Why is this needed?
    CRITICAL_SECTION_BEGIN();

    LL_GPIO_ResetOutputPin(RADIO_CS_GPIO_Port, RADIO_CS_Pin);

    HAL_SPI_Transmit(&hspi1, pucTxBuff, sizeof(pucTxBuff), 10);

    // SpiInOut( &SX126x.Spi, RADIO_GET_STATUS );
    // SpiInOut( &SX126x.Spi, 0x00 );

    LL_GPIO_SetOutputPin(RADIO_CS_GPIO_Port, RADIO_CS_Pin);

    // Wait for chip to be ready.
    SX126xWaitOnBusy( );

    // Update operating mode context variable
    SX126xSetOperatingMode( MODE_STDBY_RC );

    CRITICAL_SECTION_END();
}

void SX126xWriteCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
    SX126xCheckDeviceReady( );

    LL_GPIO_ResetOutputPin(RADIO_CS_GPIO_Port, RADIO_CS_Pin);

    HAL_SPI_Transmit(&hspi1, (uint8_t *)&command, 1, 10);
    // SpiInOut( &SX126x.Spi, ( uint8_t )command );

    // for( uint16_t i = 0; i < size; i++ )
    // {
    //     SpiInOut( &SX126x.Spi, buffer[i] );
    // }
    HAL_SPI_Transmit(&hspi1, buffer, size, 10);

    LL_GPIO_SetOutputPin(RADIO_CS_GPIO_Port, RADIO_CS_Pin);

    if( command != RADIO_SET_SLEEP )
    {
        SX126xWaitOnBusy( );
    }
}

uint8_t SX126xReadCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
    uint8_t status = 0;

    SX126xCheckDeviceReady( );

    LL_GPIO_ResetOutputPin(RADIO_CS_GPIO_Port, RADIO_CS_Pin);

    // SpiInOut( &SX126x.Spi, ( uint8_t )command );
    // status = SpiInOut( &SX126x.Spi, 0x00 );

    HAL_SPI_Transmit(&hspi1, (uint8_t *)&command, 1, 10);
    HAL_SPI_Receive(&hspi1, &status, 1, 10);

    // for( uint16_t i = 0; i < size; i++ )
    // {
    //     buffer[i] = SpiInOut( &SX126x.Spi, 0 );
    // }
    HAL_SPI_Receive(&hspi1, buffer, size, 10);

    LL_GPIO_SetOutputPin(RADIO_CS_GPIO_Port, RADIO_CS_Pin);

    SX126xWaitOnBusy( );

    return status;
}

void SX126xWriteRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
    SX126xCheckDeviceReady( );

    LL_GPIO_ResetOutputPin(RADIO_CS_GPIO_Port, RADIO_CS_Pin);

    // SpiInOut( &SX126x.Spi, RADIO_WRITE_REGISTER );
    // SpiInOut( &SX126x.Spi, ( address & 0xFF00 ) >> 8 );
    // SpiInOut( &SX126x.Spi, address & 0x00FF );
    uint8_t pucTxBuff[] = {RADIO_WRITE_REGISTER, ( address & 0xFF00 ) >> 8, address & 0x00FF};
    HAL_SPI_Transmit(&hspi1, pucTxBuff, sizeof(pucTxBuff), 10);

    // for( uint16_t i = 0; i < size; i++ )
    // {
    //     SpiInOut( &SX126x.Spi, buffer[i] );
    // }
    HAL_SPI_Transmit(&hspi1, buffer, size, 10);

    LL_GPIO_SetOutputPin(RADIO_CS_GPIO_Port, RADIO_CS_Pin);

    SX126xWaitOnBusy( );
}

void SX126xWriteRegister( uint16_t address, uint8_t value )
{
    SX126xWriteRegisters( address, &value, 1 );
}

void SX126xReadRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
    SX126xCheckDeviceReady( );

    LL_GPIO_ResetOutputPin(RADIO_CS_GPIO_Port, RADIO_CS_Pin);

    // SpiInOut( &SX126x.Spi, RADIO_READ_REGISTER );
    // SpiInOut( &SX126x.Spi, ( address & 0xFF00 ) >> 8 );
    // SpiInOut( &SX126x.Spi, address & 0x00FF );
    // SpiInOut( &SX126x.Spi, 0 );
    uint8_t pucTxBuff[] = {RADIO_READ_REGISTER, ( address & 0xFF00 ) >> 8, address & 0x00FF, 0};
    HAL_SPI_Transmit(&hspi1, pucTxBuff, sizeof(pucTxBuff), 10);

    // for( uint16_t i = 0; i < size; i++ )
    // {
    //     buffer[i] = SpiInOut( &SX126x.Spi, 0 );
    // }
    HAL_SPI_Receive(&hspi1, buffer, size, 10);

    LL_GPIO_SetOutputPin(RADIO_CS_GPIO_Port, RADIO_CS_Pin);

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

    LL_GPIO_ResetOutputPin(RADIO_CS_GPIO_Port, RADIO_CS_Pin);

    // SpiInOut( &SX126x.Spi, RADIO_WRITE_BUFFER );
    // SpiInOut( &SX126x.Spi, offset );
    uint8_t pucTxBuff[] = {RADIO_WRITE_BUFFER, offset};
    HAL_SPI_Transmit(&hspi1, pucTxBuff, sizeof(pucTxBuff), 10);

    // for( uint16_t i = 0; i < size; i++ )
    // {
    //     SpiInOut( &SX126x.Spi, buffer[i] );
    // }
    HAL_SPI_Transmit(&hspi1, buffer, size, 10);

    LL_GPIO_SetOutputPin(RADIO_CS_GPIO_Port, RADIO_CS_Pin);

    SX126xWaitOnBusy( );
}

void SX126xReadBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    SX126xCheckDeviceReady( );

    LL_GPIO_ResetOutputPin(RADIO_CS_GPIO_Port, RADIO_CS_Pin);

    // SpiInOut( &SX126x.Spi, RADIO_READ_BUFFER );
    // SpiInOut( &SX126x.Spi, offset );
    // SpiInOut( &SX126x.Spi, 0 );
    uint8_t pucTxBuff[] = {RADIO_READ_BUFFER, offset, 0};
    HAL_SPI_Transmit(&hspi1, pucTxBuff, sizeof(pucTxBuff), 10);

    // for( uint16_t i = 0; i < size; i++ )
    // {
    //     buffer[i] = SpiInOut( &SX126x.Spi, 0 );
    // }

    HAL_SPI_Receive(&hspi1, buffer, size, 10);

    LL_GPIO_SetOutputPin(RADIO_CS_GPIO_Port, RADIO_CS_Pin);

    SX126xWaitOnBusy( );
}

void SX126xSetRfTxPower( int8_t power )
{
    SX126xSetTxParams( power, RADIO_RAMP_40_US );
}

uint8_t SX126xGetDeviceId( void )
{
    return SX1262;
}

void SX126xAntSwOn( void )
{
    LL_GPIO_SetOutputPin(RADIO_TXEN_GPIO_Port, RADIO_TXEN_Pin);
    LL_GPIO_ResetOutputPin(RADIO_RXEN_GPIO_Port, RADIO_RXEN_Pin);
}

void SX126xAntSwOff( void )
{
    LL_GPIO_ResetOutputPin(RADIO_TXEN_GPIO_Port, RADIO_TXEN_Pin);
    LL_GPIO_SetOutputPin(RADIO_RXEN_GPIO_Port, RADIO_RXEN_Pin);
}

bool SX126xCheckRfFrequency( uint32_t frequency )
{
    (void)frequency;
    // Implement check. Currently all frequencies are supported
    return true;
}

void BoardCriticalSectionBegin( uint32_t *mask )
{
    *mask = __get_PRIMASK( );
    __disable_irq( );
}

void BoardCriticalSectionEnd( uint32_t *mask )
{
    __set_PRIMASK( *mask );
}
