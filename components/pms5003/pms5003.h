#ifndef PMS5003_COMPONENT_H
#define PMS5003_COMPONENT_H

#include "sdkconfig.h"
#include "driver/uart.h"


#define UART_PORT           CONFIG_UART_PORT
#define BAUD_RATE           CONFIG_UART_BAUD_RATE
#define RX_BUFFER_SIZE      CONFIG_UART_RX_BUFFER_SIZE
#define TX_BUFFER_SIZE      CONFIG_UART_TX_BUFFER_SIZE
#define EVENT_QUEUE_SIZE    CONFIG_UART_EVENT_QUEUE_SIZE
#define RX_PIN              CONFIG_UART_RX_PIN
#define TX_PIN              CONFIG_UART_TX_PIN

typedef struct
{
    uint16_t pm1_0_conc_cf;
    uint16_t pm2_5_conc_cf;
    uint16_t pm10_conc_cf;
    uint16_t pm1_0_conc_atmos;
    uint16_t pm2_5_conc_atmos;
    uint16_t pm10_conc_atmos;
    uint16_t particles_0_3;
    uint16_t particles_0_5;
    uint16_t particles_1_0;
    uint16_t particles_2_5;
    uint16_t particles_5_0;
    uint16_t particles_10;
    uint16_t check_code;
} pms5003_data;

extern pms5003_data pms5003Data;

void pms5003_init();
void pms5003_read_data();

#endif