# Indoor Environment monitor

---

## Description
This is a project I created to monitor conditions of living spaces. The main inspiration behind this project was to monitor indoor air quality in order to know when
to turn on air purifiers. This helps to protect people who are sensitive to poor air quality.

## Specifications
This project was created in VS Code with the ESP IDF v5.5 extension. The hardware used is listed in the table below.

| Component | Description |
| :-------: | :---------: |
| ESP32     | HiLetGo ESP32-WROOM-32D Dev Board |
| BME280    | Adafruit BME280 STEMMA QT board |
| PMS5003   | Adafruit PM2.5 Air Quality Sensor and Breadboard Adapter Kit |

## State of the Project
The BME280 is currently communicating over I2C using a custom driver and the ESP IDF HAL.

The next step is to write a custom driver for the PMS5003 sensor utilizing UART.