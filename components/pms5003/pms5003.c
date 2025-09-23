#include "pms5003.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static void wait_for_data();

QueueHandle_t uart_queue;
pms5003_data pms5003Data;

static void wait_for_data()
{
    // Wait for new data to be put in the RX buffer
    printf("Waitng for data...\n");
    int length = 0;
    while (length == 0)
    {
        ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_PORT,(size_t *)&length));
        vTaskDelay(10 / portTICK_PERIOD_MS);
        
    }
    printf("Bytes available: %i\n", length);
}

void pms5003_init()
{
    // Install driver
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, RX_BUFFER_SIZE, TX_BUFFER_SIZE, 0, NULL, 0));

    // Define UART Config
    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));

    // Set Pins
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, TX_PIN, RX_PIN, -1, -1));
}

void pms5003_read_data()
{
    // Start by removing any old data
    uart_flush(UART_PORT);

    wait_for_data();

    // Data is in the buffer. Now read it.
    char buffer[32];
    char temp = 0x00;

    // Look for start byte 1
    while (temp != 0x42)
    {
        printf("temp: %c\n", temp);
        uart_read_bytes(UART_PORT, (void *)&temp, 1, 100);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    // Found start byte 1, read rest of data in.
    buffer[0] = temp;
    uart_read_bytes(UART_PORT, buffer+1, 31, 100);

    pms5003Data.pm1_0_conc_cf = buffer[4] << 8 | buffer[5];
    pms5003Data.pm2_5_conc_cf = buffer[6] << 8 | buffer[7];
    pms5003Data.pm10_conc_cf = buffer[8] << 8 | buffer[9];
    pms5003Data.pm1_0_conc_atmos = buffer[10] << 8 | buffer[11];
    pms5003Data.pm2_5_conc_atmos = buffer[12] << 8 | buffer[13];
    pms5003Data.pm10_conc_atmos = buffer[14] << 8 | buffer[15];
    pms5003Data.particles_0_3 = buffer[16] << 8 | buffer[17];
    pms5003Data.particles_0_5 = buffer[18] << 8 | buffer[19];
    pms5003Data.particles_1_0 = buffer[20] << 8 | buffer[21];
    pms5003Data.particles_2_5 = buffer[22] << 8 | buffer[23];
    pms5003Data.particles_5_0 = buffer[24] << 8 | buffer[25];
    pms5003Data.particles_10 = buffer[26] << 8 | buffer[27];
}