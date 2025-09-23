/*
BME280.h

Author: William Cobb
Created on: May 12, 2025

*/
#ifndef BME280_COMPONENT_H
#define BME280_COMPONENT_H

#include "driver/i2c_master.h"

// Device registers
enum
{

    BME280_I2C_ADDR = 0x76,

    BME280_REG_ID = 0xD0,
    BME280_REG_RESET = 0xE0,
    BME280_REG_CTRLHUM = 0xF2,
    BME280_REG_STATUS = 0xF3,
    BME280_REG_CTRLMEAS = 0xF4,
    BME280_REG_CONFIG = 0xF5,
    BME280_REG_PRESS = 0xF7,
    BME280_REG_TEMP = 0xFA,
    BME280_REG_HUM = 0xFD,

    // Calibration registers
    BME280_CALIB_DIG_T1 = 0x88,
    BME280_CALIB_DIG_T2 = 0x8A,
    BME280_CALIB_DIG_T3 = 0x8C,
    BME280_CALIB_DIG_P1 = 0x8E,
    BME280_CALIB_DIG_P2 = 0x90,
    BME280_CALIB_DIG_P3 = 0x92,
    BME280_CALIB_DIG_P4 = 0x94,
    BME280_CALIB_DIG_P5 = 0x96,
    BME280_CALIB_DIG_P6 = 0x98,
    BME280_CALIB_DIG_P7 = 0x9A,
    BME280_CALIB_DIG_P8 = 0x9C,
    BME280_CALIB_DIG_P9 = 0x9E,
    BME280_CALIB_DIG_H1 = 0xA1,
    BME280_CALIB_DIG_H2 = 0xE1,
    BME280_CALIB_DIG_H3 = 0xE3,
    BME280_CALIB_DIG_H4 = 0xE4, // 0xE5[3:0]
    BME280_CALIB_DIG_H5 = 0xE5, // 0xE5[7:4]
    BME280_CALIB_DIG_H6 = 0xE7
};

enum sample_rate
{
    skipped = 0x0,
    sample_x1 = 0x1,
    sample_x2 = 0x2,
    sample_x4 = 0x3,
    sample_x8 = 0x4,
    sample_x16 = 0x5
};

enum sensor_mode
{
    sleep = 0x0,
    forced = 0x1,
    normal = 0x3,
};


enum standby_time
{
    sb0_5 = 0x0,
    sb62_5 = 0x1,
    sb125 = 0x2,
    sb250 = 0x3,
    sb500 = 0x4,
    sb1000 = 0x5,
    sb10 = 0x6,
    sb20 = 0x7
};

enum filter_coefficient
{
    filter_off = 0x0,
    filter_2 = 0x1,
    filter_4 = 0x2,
    filter_8 = 0x3,
    filter_16 = 0x4
};

typedef struct
{
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
    uint8_t dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
    int8_t dig_H6;
} bme280_calib_data;

typedef struct
{
    uint8_t id;
    uint8_t osrs_h;
    uint8_t osrs_t;
    uint8_t osrs_p;
    uint8_t mode;
    uint8_t t_sb;
    uint8_t filter;
    uint8_t spi32_en;
} bme280_config;

typedef struct
{
    float temp;
    float press;
    float hum;
} bme280_data;

typedef int32_t BME280_S32_t;
typedef uint32_t BME280_U32_t;
typedef int64_t BME280_S64_t;

extern bme280_data bme280data;

void bme280_init(i2c_master_dev_handle_t *dev_handle);
void bme280_read_data();




#endif