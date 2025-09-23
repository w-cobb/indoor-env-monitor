#include "bme280.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <portmacro.h>

// Static function declarations

static void readCalibData();
static uint8_t readRegister8(uint8_t reg);
static uint16_t readRegister16(uint8_t reg);
static void burstReadData(uint8_t *buffer);
static void writeRegister(uint8_t reg, uint8_t data);
static void writeConfig();
static float compensateT(BME280_S32_t adc_T);
static float compensateH(BME280_S32_t adc_H);
static float compensateP(BME280_S32_t adc_P);

bme280_data bme280data;
bme280_config config;

i2c_master_dev_handle_t _dev_handle;
bme280_calib_data _calib;

BME280_S32_t _t_fine;

/*
Initialize the sensor with the device handle for I2C communication, read the callibration registers,
and perform inital configuration of the sensor.
*/
void bme280_init(i2c_master_dev_handle_t *dev_handle) {
    printf("Assigning handle\n");
    _dev_handle = *dev_handle;

    // Wait for power-on-reset to complete
    printf("Waiting for power-on-resset to finish...\n");
    while (readRegister8(BME280_REG_ID) != 0x60);
    printf("Fetched the sensor ID.\n");

    printf("Reading calibration registers\n");
    readCalibData();
    printf("Done\n");

    printf("Writing to config registers\n");
    writeConfig();
    printf("Config written\n");

    // Configure
    config.id = 0x60;
    config.osrs_h = sample_x1;
    config.osrs_t = sample_x1;
    config.osrs_p = sample_x1;
    config.mode = sleep;
    config.t_sb = sb1000;
    config.filter = filter_off;
    config.spi32_en = 0;
    
    bme280data.temp = 0.0f;
    bme280data.press = 0.0f;
    bme280data.hum = 0.0f;
}

/*
Read data into the data struct.
*/
void bme280_read_data() {
    // Set sensor mode to forced and wait for data to be available.
    printf("Entering forced mode\n");
    config.mode = forced;
    writeConfig();
    printf("Forced mode on\n");
    while ((readRegister8(BME280_REG_STATUS) & 0x8) == 0x8);
    printf("Data is now available\n");

    // Burst read the data registers
    uint8_t buffer[8];
    burstReadData(buffer);

    printf("Data registers have been read\n");
    // Run the compensation formulas on the data values
    BME280_S32_t adc_P, adc_T, adc_H;

    // Read adc values from buffer
    adc_P = buffer[0] << 12 | buffer[1] << 4 | (buffer[2] & 0xF);
    adc_T = buffer[3] << 12 | buffer[4] << 4 | (buffer[5] & 0xF);
    adc_H = buffer[6] << 8 | buffer[7];


    // Convert adc values to float and store them in data struct.
    bme280data.temp = compensateT(adc_T) / 100.0f;
    bme280data.press = compensateP(adc_P) / 256 / 100.0f;
    bme280data.hum = compensateH(adc_H) / 1024.0f;
}

/*
Read the calibration registers into the _calib struct.
*/
static void readCalibData() {
    // Read calibration data registers
    _calib.dig_T1 = readRegister16(BME280_CALIB_DIG_T1);
    _calib.dig_T2 = (int16_t)readRegister16(BME280_CALIB_DIG_T2);
    _calib.dig_T3 = (int16_t)readRegister16(BME280_CALIB_DIG_T3);
    _calib.dig_P1 = readRegister16(BME280_CALIB_DIG_P1);
    _calib.dig_P2 = (int16_t)readRegister16(BME280_CALIB_DIG_P2);
    _calib.dig_P3 = (int16_t)readRegister16(BME280_CALIB_DIG_P3);
    _calib.dig_P4 = (int16_t)readRegister16(BME280_CALIB_DIG_P4);
    _calib.dig_P5 = (int16_t)readRegister16(BME280_CALIB_DIG_P5);
    _calib.dig_P6 = (int16_t)readRegister16(BME280_CALIB_DIG_P6);
    _calib.dig_P7 = (int16_t)readRegister16(BME280_CALIB_DIG_P7);
    _calib.dig_P8 = (int16_t)readRegister16(BME280_CALIB_DIG_P8);
    _calib.dig_P9 = (int16_t)readRegister16(BME280_CALIB_DIG_P9);
    _calib.dig_H1 = readRegister8(BME280_CALIB_DIG_H1);
    _calib.dig_H2 = (int16_t)readRegister16(BME280_CALIB_DIG_H2);
    _calib.dig_H3 = readRegister8(BME280_CALIB_DIG_H3);
    _calib.dig_H4 = (int16_t)(readRegister8(BME280_CALIB_DIG_H4) << 4 | (readRegister8(BME280_CALIB_DIG_H5) & 0xF));
    _calib.dig_H5 = (int16_t)(readRegister8(0xE6) << 4 | (readRegister8(BME280_CALIB_DIG_H5) & 0xF));
    _calib.dig_H6 = (int8_t)readRegister8(BME280_CALIB_DIG_H6);
}

/**
Write changes to configuration registers. Configuration is stored in the "config" struct.
*/
static void writeConfig() {
    // Write configuration
    writeRegister(BME280_REG_CTRLHUM, config.osrs_h);
    uint8_t temp = config.osrs_t << 5 | config.osrs_p << 2 | config.mode; // ctrl_meas
    writeRegister(BME280_REG_CTRLMEAS, temp);
    temp = config.t_sb << 5 | config.filter << 2 | config.spi32_en; // config
    writeRegister(BME280_REG_CONFIG, temp);
}

/*
Read an unsigned char from given register address.
*/
static uint8_t readRegister8(uint8_t reg) {
    uint8_t temp;
    i2c_master_transmit_receive(_dev_handle, &reg, 1, &temp, 1, 1000 / portTICK_PERIOD_MS);
    return temp;
}

/*
Read an unsigned short from given register address.
*/
static uint16_t readRegister16(uint8_t reg) {
    uint8_t temp[2];
    i2c_master_transmit_receive(_dev_handle, &reg, 1, temp, 2, 1000 / portTICK_PERIOD_MS);
    return (uint16_t)(temp[1] << 8 | temp[0]);
}

/*
Read from all data registers at once (burst read). This is recommended to prevent inconsistent data read between measurement cycles.
*/
static void burstReadData(uint8_t *buffer) {
    // Read from all data registers at once
    printf("Reading data registers...\n");
    uint8_t reg = BME280_REG_PRESS;
    ESP_ERROR_CHECK(i2c_master_transmit(_dev_handle, &reg, 1, -1));
    ESP_ERROR_CHECK(i2c_master_receive(_dev_handle, buffer, 8, -1));
    printf("Done\n");
}

/*
Write an unsigned char to register at the given address. This is performed by sending pairs of register addresses and data. i.e. write [reg, data]
*/
static void writeRegister(uint8_t reg, uint8_t data) {
    uint8_t buffer[2] = {reg, data};
    i2c_master_transmit(_dev_handle, buffer, 2, 1000 / portTICK_PERIOD_MS);
}

/*
Convert adc temperature value to float using the formula given in section 4.2.3 of the BME280 datasheet.
*/
static float compensateT(BME280_S32_t adc_T) {
    BME280_S32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((BME280_S32_t)_calib.dig_T1 << 1))) * ((BME280_S32_t)_calib.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((BME280_S32_t)_calib.dig_T1)) * ((adc_T >> 4) - ((BME280_S32_t)_calib.dig_T1))) >> 12) *
            ((BME280_S32_t)_calib.dig_T3)) >> 14;
    _t_fine = var1 + var2;
    T = (_t_fine * 5 + 128) >> 8;
    return (float)T;
}

/*
Convert adc pressure value to float using the formula given in section 4.2.3 of the BME280 datasheet.
*/
static float compensateP(BME280_S32_t adc_P) {
    BME280_S64_t var1, var2, p;
    var1 = ((BME280_S64_t)_t_fine) - 128000;
    var2 = var1 * var1 * (BME280_S64_t)_calib.dig_P6;
    var2 = var2 + ((var1 * (BME280_S64_t)_calib.dig_P5) << 17);
    var2 = var2 + (((BME280_S64_t)_calib.dig_P4) << 35);
    var1 = ((var1 * var1 * (BME280_S64_t)_calib.dig_P3) >> 8) + ((var1 * (BME280_S64_t)_calib.dig_P2) << 12);
    var1 = (((((BME280_S64_t)1) << 47) + var1)) * ((BME280_S64_t)_calib.dig_P1) >> 33;
    if (var1 == 0)
    {
        return 0; // avoid exception caused by division by zero
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((BME280_S64_t)_calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((BME280_S64_t)_calib.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((BME280_S64_t)_calib.dig_P7) << 4);
    return (float)p;
}

/*
Convert adc humidity value to float using the formula given in section 4.2.3 of the BME280 datasheet.
*/
static float compensateH(BME280_S32_t adc_H) {
    BME280_S32_t v_x1_u32r;
    v_x1_u32r = (_t_fine - ((BME280_S32_t)76800));
    v_x1_u32r = (_t_fine - ((BME280_S32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((BME280_S32_t)_calib.dig_H4) << 20) - (((BME280_S32_t)_calib.dig_H5) * v_x1_u32r)) 
        + ((BME280_S32_t)16384)) >> 15) * (((((((v_x1_u32r * ((BME280_S32_t)_calib.dig_H6)) >> 10) 
        * (((v_x1_u32r * ((BME280_S32_t)_calib.dig_H3)) >> 11) + ((BME280_S32_t)32768))) >> 10) 
        + ((BME280_S32_t)2097152)) * ((BME280_S32_t)_calib.dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((BME280_S32_t)_calib.dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    return (BME280_U32_t)(v_x1_u32r >> 12);
    v_x1_u32r = (((((adc_H << 14) - (((BME280_S32_t)_calib.dig_H4) << 20) - (((BME280_S32_t)_calib.dig_H5) * v_x1_u32r)) 
        + ((BME280_S32_t)16384)) >> 15) * (((((((v_x1_u32r * ((BME280_S32_t)_calib.dig_H6)) >> 10) 
        * (((v_x1_u32r * ((BME280_S32_t)_calib.dig_H3)) >> 11) + ((BME280_S32_t)32768))) >> 10) 
        + ((BME280_S32_t)2097152)) * ((BME280_S32_t)_calib.dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((BME280_S32_t)_calib.dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    return (float)(v_x1_u32r >> 12);
}
