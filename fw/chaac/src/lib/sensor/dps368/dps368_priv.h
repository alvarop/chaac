#ifndef __DPS368_PRIV_H__
#define __DPS368_PRIV_H__

typedef struct
{
    uint8_t regAddress;
    uint8_t mask;
    uint8_t shift;
} RegMask_t;

typedef struct
{
    uint8_t regAddress;
    uint8_t length;
} RegBlock_t;


#define DPS368_NUM_OF_REGMASKS 16

enum Interrupt_source_368_e
{
    DPS368_NO_INTR = 0,
    DPS368_PRS_INTR = 1,
    DPS368_TEMP_INTR = 2,
    DPS368_BOTH_INTR = 3,
    DPS368_FIFO_FULL_INTR = 4,
};

enum Registers_e
{
    PROD_ID = 0,
    REV_ID,
    TEMP_SENSOR,    // internal vs external
    TEMP_SENSORREC, //temperature sensor recommendation
    TEMP_SE,        //temperature shift enable (if temp_osr>3)
    PRS_SE,         //pressure shift enable (if prs_osr>3)
    FIFO_FL,        //FIFO flush
    FIFO_EMPTY,     //FIFO empty
    FIFO_FULL,      //FIFO full
    INT_HL,
    INT_SEL,         //interrupt select
};

const RegMask_t registers[DPS368_NUM_OF_REGMASKS] = {
    {0x0D, 0x0F, 0}, // PROD_ID
    {0x0D, 0xF0, 4}, // REV_ID
    {0x07, 0x80, 7}, // TEMP_SENSOR
    {0x28, 0x80, 7}, // TEMP_SENSORREC
    {0x09, 0x08, 3}, // TEMP_SE
    {0x09, 0x04, 2}, // PRS_SE
    {0x0C, 0x80, 7}, // FIFO_FL
    {0x0B, 0x01, 0}, // FIFO_EMPTY
    {0x0B, 0x02, 1}, // FIFO_FULL
    {0x09, 0x80, 7}, // INT_HL
    {0x09, 0x70, 4}, // INT_SEL
};

const RegBlock_t coeffBlock = {0x10, 18};

#define DPS__FIFO_SIZE 32
#define DPS__STD_SLAVE_ADDRESS 0x77U
#define DPS__RESULT_BLOCK_LENGTH 3
#define NUM_OF_COMMON_REGMASKS 16

#define DPS__MEASUREMENT_RATE_1 0
#define DPS__MEASUREMENT_RATE_2 1
#define DPS__MEASUREMENT_RATE_4 2
#define DPS__MEASUREMENT_RATE_8 3
#define DPS__MEASUREMENT_RATE_16 4
#define DPS__MEASUREMENT_RATE_32 5
#define DPS__MEASUREMENT_RATE_64 6
#define DPS__MEASUREMENT_RATE_128 7

#define DPS__OVERSAMPLING_RATE_1 DPS__MEASUREMENT_RATE_1
#define DPS__OVERSAMPLING_RATE_2 DPS__MEASUREMENT_RATE_2
#define DPS__OVERSAMPLING_RATE_4 DPS__MEASUREMENT_RATE_4
#define DPS__OVERSAMPLING_RATE_8 DPS__MEASUREMENT_RATE_8
#define DPS__OVERSAMPLING_RATE_16 DPS__MEASUREMENT_RATE_16
#define DPS__OVERSAMPLING_RATE_32 DPS__MEASUREMENT_RATE_32
#define DPS__OVERSAMPLING_RATE_64 DPS__MEASUREMENT_RATE_64
#define DPS__OVERSAMPLING_RATE_128 DPS__MEASUREMENT_RATE_128

//we use 0.1 ms units for time calculations, so 10 units are one millisecond
#define DPS__BUSYTIME_SCALING 10U

// DPS310 has 10 milliseconds of spare time for each synchronous measurement / per second for asynchronous measurements
// this is for error prevention on friday-afternoon-products :D
// you can set it to 0 if you dare, but there is no warranty that it will still work
#define DPS310__BUSYTIME_FAILSAFE 10U
#define DPS310__MAX_BUSYTIME ((1000U - DPS310__BUSYTIME_FAILSAFE) * DPS__BUSYTIME_SCALING)

#define DPS__NUM_OF_SCAL_FACTS 8

// status code
#define DPS__SUCCEEDED 0
#define DPS__FAIL_UNKNOWN -1
#define DPS__FAIL_INIT_FAILED -2
#define DPS__FAIL_TOOBUSY -3
#define DPS__FAIL_UNFINISHED -4

/**
 * @brief Operating mode.
 * 
 */
enum Mode
{
    IDLE = 0x00,
    CMD_PRS = 0x01,
    CMD_TEMP = 0x02,
    CMD_BOTH = 0x03, // only for DPS422
    CONT_PRS = 0x05,
    CONT_TMP = 0x06,
    CONT_BOTH = 0x07
};

enum RegisterBlocks_e
{
    PRS = 0, // pressure value
    TEMP,    // temperature value
};

const RegBlock_t registerBlocks[2] = {
    {0x00, 3},
    {0x03, 3},
};

/**
 * @brief registers for configuration and flags; these are the same for both 310 and 422, might need to be adapted for future sensors
 * 
 */
enum Config_Registers_e
{
    TEMP_MR = 0, // temperature measure rate
    TEMP_OSR,    // temperature measurement resolution
    PRS_MR,      // pressure measure rate
    PRS_OSR,     // pressure measurement resolution
    MSR_CTRL,    // measurement control
    FIFO_EN,

    TEMP_RDY,
    PRS_RDY,
    INT_FLAG_FIFO,
    INT_FLAG_TEMP,
    INT_FLAG_PRS,
};

const RegMask_t config_registers[NUM_OF_COMMON_REGMASKS] = {
    {0x07, 0x70, 4}, // TEMP_MR
    {0x07, 0x07, 0}, // TEMP_OSR
    {0x06, 0x70, 4}, // PRS_MR
    {0x06, 0x07, 0}, // PRS_OSR
    {0x08, 0x07, 0}, // MSR_CTRL
    {0x09, 0x02, 1}, // FIFO_EN

    {0x08, 0x20, 5}, // TEMP_RDY
    {0x08, 0x10, 4}, // PRS_RDY
    {0x0A, 0x04, 2}, // INT_FLAG_FIFO
    {0x0A, 0x02, 1}, // INT_FLAG_TEMP
    {0x0A, 0x01, 0}, // INT_FLAG_PRS
};

#endif

