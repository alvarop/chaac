
#include "FreeRTOS.h"
#include "task.h"
#include "dps368.h"
#include "dps368_priv.h"
#include "io_i2c.h"

static int16_t readByte(uint8_t regAddress);
static int16_t writeByte(uint8_t regAddress, uint8_t data, uint8_t check); 
static int16_t readByteBitfield(RegMask_t regMask); 
static int16_t correctTemp();
static uint16_t calcBusyTime(uint16_t mr, uint16_t osr); 

static int16_t writeByteBitfieldReg(uint8_t data, RegMask_t regMask);
static int16_t writeByteBitfield(uint8_t data, uint8_t regAddress, uint8_t mask, uint8_t shift, uint8_t check);
static int16_t readBlock(RegBlock_t regBlock, uint8_t *buffer);
static int16_t readcoeffs();
static void getTwosComplement(int32_t *raw, uint8_t length);
static int16_t getSingleResult(float *result); 
static int16_t startMeasureTempOnce(uint8_t oversamplingRate);
static int16_t startMeasurePressureOnce(uint8_t oversamplingRate);
static int16_t getRawResult(int32_t *raw, RegBlock_t reg); 
static float calcTemp(int32_t raw);
static float calcPressure(int32_t raw);

static enum Mode m_opMode;
static uint8_t m_tempSensor;

static uint8_t m_initFail;

static uint8_t m_productID;
static uint8_t m_revisionID;

static uint8_t m_tempMr;
static uint8_t m_tempOsr;
static uint8_t m_prsMr;
static uint8_t m_prsOsr;

static int32_t m_c00;
static int32_t m_c10;
static int32_t m_c01;
static int32_t m_c11;
static int32_t m_c20;
static int32_t m_c21;
static int32_t m_c30;

static int32_t m_c0Half;
static int32_t m_c1;

const int32_t scaling_facts[DPS__NUM_OF_SCAL_FACTS] = {524288, 1572864, 3670016, 7864320, 253952, 516096, 1040384, 2088960};

// last measured scaled temperature (necessary for pressure compensation)
static float m_lastTempScal;

void *pvI2CHandle = NULL;

int32_t dps368_init(void * pvI2CDevHandle) {
    configASSERT(pvI2CDevHandle != NULL);
    pvI2CHandle = pvI2CDevHandle;

    m_initFail = 1;

    int16_t prodId = readByteBitfield(registers[PROD_ID]);
    if (prodId < 0) {
        return -1;
    }
    m_productID = prodId;

    int16_t revId = readByteBitfield(registers[REV_ID]);
    if (revId < 0) {
        return -1;
    }
    m_revisionID = revId;

    int16_t sensor = readByteBitfield(registers[TEMP_SENSORREC]);
    m_tempSensor = sensor;

    if(writeByteBitfieldReg((uint8_t)sensor, registers[TEMP_SENSOR]) < 0) {
        return -1;
    }

    if(readcoeffs() < 0) {
        return -1;
    }
    
    dps368_standby();

    dps368_config_temp(DPS__MEASUREMENT_RATE_4, DPS__OVERSAMPLING_RATE_8);
    dps368_config_pressure(DPS__MEASUREMENT_RATE_4, DPS__OVERSAMPLING_RATE_8);

    float trash;
    dps368_measure_temp_once(&trash);

    dps368_standby();

    correctTemp();

    m_initFail = 0;

    return 0;
}

int16_t dps368_standby(void) {
    if (m_initFail) {
        return DPS__FAIL_INIT_FAILED;
    }

    int16_t ret = dps368_set_op_mode(IDLE);
    if (ret != DPS__SUCCEEDED) {
        return ret;
    }

    return dps368_disable_fifo();
}

int16_t dps368_set_op_mode(uint8_t opMode) {
    if (writeByteBitfieldReg(opMode, config_registers[MSR_CTRL]) == -1) {
        return DPS__FAIL_UNKNOWN;
    }
    m_opMode = (enum Mode)opMode;
    return DPS__SUCCEEDED;
}

int16_t dps368_config_temp(uint8_t tempMr, uint8_t tempOsr) {
    tempMr &= 0x07;
    tempOsr &= 0x07;

    int16_t ret = writeByteBitfieldReg(tempMr, config_registers[TEMP_MR]);
    if (ret != DPS__SUCCEEDED) {
        return DPS__FAIL_UNKNOWN;
    }

    ret = writeByteBitfieldReg(tempOsr, config_registers[TEMP_OSR]);
    if (ret != DPS__SUCCEEDED) {
        return DPS__FAIL_UNKNOWN;
    }
    m_tempMr = tempMr;
    m_tempOsr = tempOsr;

    return ret;
}

int16_t dps368_config_pressure(uint8_t prsMr, uint8_t prsOsr) {
    prsMr &= 0x07;
    prsOsr &= 0x07;

    int16_t ret = writeByteBitfieldReg(prsMr, config_registers[PRS_MR]);
    if (ret != DPS__SUCCEEDED) {
        return DPS__FAIL_UNKNOWN;
    }

    ret = writeByteBitfieldReg(prsOsr, config_registers[PRS_OSR]);

    if (ret != DPS__SUCCEEDED) {
        return DPS__FAIL_UNKNOWN;
    }
    m_prsMr = prsMr;
    m_prsOsr = prsOsr;

    return ret;
}
int16_t dps368_enable_fifo() {
    return writeByteBitfieldReg(1, config_registers[FIFO_EN]);
}

int16_t dps368_disable_fifo() {
    int16_t ret = dps368_flush_fifo();
    if (ret != DPS__SUCCEEDED) {
        return DPS__FAIL_UNKNOWN;
    }

    return writeByteBitfieldReg(0, config_registers[FIFO_EN]);
}

int16_t dps368_flush_fifo() {
    return writeByteBitfieldReg(1, registers[FIFO_FL]);
}



int16_t dps368_measure_temp_once(float *result) {
    //Start measurement

    int16_t ret = startMeasureTempOnce(m_tempOsr);
    if (ret != DPS__SUCCEEDED) {
        return ret;
    }

    uint32_t delay_ticks = pdMS_TO_TICKS((calcBusyTime(0U, m_tempOsr) / DPS__BUSYTIME_SCALING) + 
                DPS310__BUSYTIME_FAILSAFE);
    if (delay_ticks < 2) {
        delay_ticks = 2;
    }
    vTaskDelay(delay_ticks);
    

    ret = getSingleResult(result);
    if (ret != DPS__SUCCEEDED) {
        dps368_standby();
    }
    return ret;
}

int16_t dps368_measure_pressure_once(float *result) {
    //Start measurement
    int16_t ret = startMeasurePressureOnce(m_prsOsr);
    if (ret != DPS__SUCCEEDED) {
        return ret;
    }

    uint32_t delay_ticks = pdMS_TO_TICKS((calcBusyTime(0U, m_tempOsr) / DPS__BUSYTIME_SCALING) + 
                DPS310__BUSYTIME_FAILSAFE);
    if (delay_ticks < 2) {
        delay_ticks = 2;
    }
    vTaskDelay(delay_ticks);

    ret = getSingleResult(result);
    if (ret != DPS__SUCCEEDED) {
        dps368_standby();
    }
    return ret;
}


static int16_t getSingleResult(float *result) {

    //abort if initialization failed
    if (m_initFail)
    {
        return DPS__FAIL_INIT_FAILED;
    }

    if (result == NULL) {
        return DPS__FAIL_UNKNOWN;
    }

    //read finished bit for current opMode
    int16_t rdy;
    switch (m_opMode)
    {
    case CMD_TEMP: //temperature
        rdy = readByteBitfield(config_registers[TEMP_RDY]);
        break;
    case CMD_PRS: //pressure
        rdy = readByteBitfield(config_registers[PRS_RDY]);
        break;
    default: //DPS310 not in command mode
        return DPS__FAIL_TOOBUSY;
    }
    //read new measurement result
    switch (rdy)
    {
    case DPS__FAIL_UNKNOWN: //could not read ready flag
        return DPS__FAIL_UNKNOWN;
    case 0: //ready flag not set, measurement still in progress
        return DPS__FAIL_UNFINISHED;
    case 1: { //measurement ready, expected case
            enum Mode oldMode = m_opMode;
            m_opMode = IDLE; //opcode was automatically reseted by DPS310
            int32_t raw_val;
            switch (oldMode)
            {
            case CMD_TEMP: //temperature
                getRawResult(&raw_val, registerBlocks[TEMP]);
                *result = calcTemp(raw_val);
                return DPS__SUCCEEDED; // TODO
            case CMD_PRS:              //pressure
                getRawResult(&raw_val, registerBlocks[PRS]);
                *result = calcPressure(raw_val);
                return DPS__SUCCEEDED; // TODO
            default:
                return DPS__FAIL_UNKNOWN; //should already be filtered above
            }
        }
    }
    return DPS__FAIL_UNKNOWN;
}

static int16_t getRawResult(int32_t *raw, RegBlock_t reg) {
    uint8_t buffer[DPS__RESULT_BLOCK_LENGTH] = {0};
    if (readBlock(reg, buffer) != DPS__RESULT_BLOCK_LENGTH)
        return DPS__FAIL_UNKNOWN;

    *raw = (uint32_t)buffer[0] << 16 | (uint32_t)buffer[1] << 8 | (uint32_t)buffer[2];
    getTwosComplement(raw, 24);
    return DPS__SUCCEEDED;
}

static float calcTemp(int32_t raw) {
	float temp = raw;

	//scale temperature according to scaling table and oversampling
	temp /= scaling_facts[m_tempOsr];

	//update last measured temperature
	//it will be used for pressure compensation
	m_lastTempScal = temp;

	//Calculate compensated temperature
	temp = m_c0Half + m_c1 * temp;

	return temp;
}

static float calcPressure(int32_t raw) {
	float prs = raw;

	//scale pressure according to scaling table and oversampling
	prs /= scaling_facts[m_prsOsr];

	//Calculate compensated pressure
	prs = m_c00 + prs * (m_c10 + prs * (m_c20 + prs * m_c30)) + m_lastTempScal * (m_c01 + prs * (m_c11 + prs * m_c21));

	//return pressure
	return prs;
}

static int16_t startMeasureTempOnce(uint8_t oversamplingRate) {
    //abort if initialization failed
    if (m_initFail)
    {
        return DPS__FAIL_INIT_FAILED;
    }
    //abort if device is not in idling mode
    if (m_opMode != IDLE)
    {
        return DPS__FAIL_TOOBUSY;
    }

    if (oversamplingRate != m_tempOsr)
    {
        //configuration of oversampling rate
        if (dps368_config_temp(0U, oversamplingRate) != DPS__SUCCEEDED)
        {
            return DPS__FAIL_UNKNOWN;
        }
    }

    //set device to temperature measuring mode
    return dps368_set_op_mode(CMD_TEMP);
}

static int16_t startMeasurePressureOnce(uint8_t oversamplingRate) {
    //abort if initialization failed
    if (m_initFail)
    {
        return DPS__FAIL_INIT_FAILED;
    }
    //abort if device is not in idling mode
    if (m_opMode != IDLE)
    {
        return DPS__FAIL_TOOBUSY;
    }

    if (oversamplingRate != m_prsOsr)
    {
        //configuration of oversampling rate
        if (dps368_config_pressure(0U, oversamplingRate) != DPS__SUCCEEDED)
        {
            return DPS__FAIL_UNKNOWN;
        }
    }

    //set device to temperature measuring mode
    return dps368_set_op_mode(CMD_PRS);
}


static uint16_t calcBusyTime(uint16_t mr, uint16_t osr) {
    //formula from datasheet (optimized)
    return ((uint32_t)20U << mr) + ((uint32_t)16U << (osr + mr));
}

static int16_t correctTemp() {
    if (m_initFail) {
        return DPS__FAIL_INIT_FAILED;
    }

    writeByte(0x0E, 0xA5, 0);
    writeByte(0x0F, 0x96, 0);
    writeByte(0x62, 0x02, 0);
    writeByte(0x0E, 0x00, 0);
    writeByte(0x0F, 0x00, 0);

    float trash;
    dps368_measure_temp_once(&trash);

    return DPS__SUCCEEDED;
}

static int16_t readByteBitfield(RegMask_t regMask) {
    int16_t ret = readByte(regMask.regAddress);
    
    if (ret < 0) {
        return ret;
    }
    return (((uint8_t)ret) & regMask.mask) >> regMask.shift;
}

static int16_t writeByteBitfieldReg(uint8_t data, RegMask_t regMask) {
    return writeByteBitfield(data, regMask.regAddress, regMask.mask, regMask.shift, 0U);
}

static int16_t writeByteBitfield(uint8_t data, uint8_t regAddress, uint8_t mask, uint8_t shift, uint8_t check) {
    int16_t old = readByte(regAddress);
    
    if (old < 0) {
        return old;
    }
    return writeByte(regAddress, ((uint8_t)old & ~mask) | ((data << shift) & mask), check);
}

static int16_t readByte(uint8_t regAddress) {
    int32_t rval = 0;
    uint8_t rBuff[1];
    uint8_t wBuff[] = {regAddress};
    
    rval = xI2CTxRx(pvI2CHandle, DPS368_ADDR, wBuff, sizeof(wBuff), rBuff, sizeof(rBuff), 10);

    if (rval != 0) {
        return DPS__FAIL_UNKNOWN;
    } else {
        return rBuff[0];
    }
}

static int16_t writeByte(uint8_t regAddress, uint8_t data, uint8_t check) {
    int32_t rval = 0;
    uint8_t wBuff[] = {regAddress, data};

    rval = xI2CTxRx(pvI2CHandle, DPS368_ADDR, wBuff, sizeof(wBuff), NULL, 0, 10);

    if (rval != 0) {
        return DPS__FAIL_UNKNOWN;
    } else {
        if (check == 0) {
            return 0;
        }
        if (readByte(regAddress) == data) {
            return DPS__SUCCEEDED;
        } else {
            return DPS__FAIL_UNKNOWN;
        }
    }
}

static int16_t readBlock(RegBlock_t regBlock, uint8_t *buffer) {
    int32_t rval = 0;
    uint8_t wBuff[] = {regBlock.regAddress};

    if (buffer == NULL) {
        return 0;
    }

    rval = xI2CTxRx(pvI2CHandle, DPS368_ADDR, wBuff, sizeof(wBuff), buffer, regBlock.length, 10);

    if (rval != 0) {
        return 0;
    } else {
        return regBlock.length;
    }
}

static int16_t readcoeffs() {
    
    uint8_t buffer[18];
    
    int16_t ret = readBlock(coeffBlock, buffer);

    if (ret != coeffBlock.length) {
        return DPS__FAIL_UNKNOWN;
    }

    //compose coefficients from buffer content
    m_c0Half = ((uint32_t)buffer[0] << 4) | (((uint32_t)buffer[1] >> 4) & 0x0F);
    getTwosComplement(&m_c0Half, 12);
    //c0 is only used as c0*0.5, so c0_half is calculated immediately
    m_c0Half = m_c0Half / 2U;

    //now do the same thing for all other coefficients
    m_c1 = (((uint32_t)buffer[1] & 0x0F) << 8) | (uint32_t)buffer[2];
    getTwosComplement(&m_c1, 12);
    m_c00 = ((uint32_t)buffer[3] << 12) | ((uint32_t)buffer[4] << 4) | (((uint32_t)buffer[5] >> 4) & 0x0F);
    getTwosComplement(&m_c00, 20);
    m_c10 = (((uint32_t)buffer[5] & 0x0F) << 16) | ((uint32_t)buffer[6] << 8) | (uint32_t)buffer[7];
    getTwosComplement(&m_c10, 20);

    m_c01 = ((uint32_t)buffer[8] << 8) | (uint32_t)buffer[9];
    getTwosComplement(&m_c01, 16);

    m_c11 = ((uint32_t)buffer[10] << 8) | (uint32_t)buffer[11];
    getTwosComplement(&m_c11, 16);
    m_c20 = ((uint32_t)buffer[12] << 8) | (uint32_t)buffer[13];
    getTwosComplement(&m_c20, 16);
    m_c21 = ((uint32_t)buffer[14] << 8) | (uint32_t)buffer[15];
    getTwosComplement(&m_c21, 16);
    m_c30 = ((uint32_t)buffer[16] << 8) | (uint32_t)buffer[17];
    getTwosComplement(&m_c30, 16);
    return DPS__SUCCEEDED;
}

static void getTwosComplement(int32_t *raw, uint8_t length) {
    if (*raw & ((uint32_t)1 << (length - 1))) {
        *raw -= (uint32_t)1 << length;
    }
}
