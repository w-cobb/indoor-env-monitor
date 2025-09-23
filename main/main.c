#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "driver/i2c.h"
#include "../components/bme280/bme280.h"
#include "../components/pms5003/pms5003.h"

#define I2C_MASTER_SCL_IO           CONFIG_I2C_MASTER_SCL
#define I2C_MASTER_SDA_IO           CONFIG_I2C_MASTER_SDA
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          CONFIG_I2C_MASTER_FREQUENCY
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0
#define I2C_MASTER_TIMEOUT_MS       1000


void initI2cBus(i2c_master_bus_handle_t *bus_handle) {
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = (gpio_num_t)I2C_MASTER_SDA_IO,
        .scl_io_num = (gpio_num_t)I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = {.enable_internal_pullup = true}
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, bus_handle));
}

void addI2cDevice(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle) {
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BME280_I2C_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &dev_config, dev_handle));
}

void app_main(void)
{
    i2c_master_bus_handle_t bus_handle;
    i2c_master_dev_handle_t bme280_handle;
    printf("Initializing I2C...\n");
    initI2cBus(&bus_handle);
    addI2cDevice(&bus_handle, &bme280_handle);
    printf("Done initializing.\n");

    bme280_init(&bme280_handle);
    pms5003_init();

    while (1) {
        printf("Reading data...\n");
        bme280_read_data();
        pms5003_read_data();
        printf("Done reading data.\n");
        printf("Temperature: %.2fC, Humidity: %.2fRH, Pressure: %.2fPa\n",
            bme280data.temp, bme280data.hum, bme280data.press);
        printf("PM1.0 Concentration: %u ug/m^3, 1.0 um Particle Count: %u\n", 
            pms5003Data.pm1_0_conc_atmos, pms5003Data.particles_1_0);
        printf("PM2.5 Concentration: %u ug/m^3, 2.5 um Particle Count: %u\n", 
            pms5003Data.pm2_5_conc_atmos, pms5003Data.particles_2_5);
        printf("PM10.0 Concentration: %u ug/m^3, 10.0 um Particle Count: %u\n", 
            pms5003Data.pm10_conc_atmos, pms5003Data.particles_10);
        
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}