# Embedded Sensor Monitoring System with SMS Alerts

This project is an embedded system that integrates various sensors with an SMS communication module. The system collects sensor data, displays it on an OLED screen, logs it to an SD card, and sends SMS alerts when a button is pressed.

## Features

- **Sensors**: BMP180 (barometer), MPU6050 (accelerometer), and various environmental sensors.
- **Display**: Adafruit SSD1306 OLED for real-time data visualization.
- **Data Logging**: Logs sensor data to an SD card in CSV format.
- **SMS Communication**: Uses SIM800L module to send SMS alerts.
- **Wireless Communication**: Integration with nRF24L01+ for potential future wireless data transmission (commented out for now).

## Hardware

- **Microcontroller**: [Your microcontroller model, e.g., STM32, Nucleo, etc.]
- **BMP180**: Barometric pressure and temperature sensor.
- **MPU6050**: 6-axis accelerometer and gyroscope sensor.
- **SSD1306**: 128x32 OLED display.
- **SIM800L**: GSM/GPRS module for SMS.
- **nRF24L01+**: Wireless transceiver module (not used in the current code).
- **SD Card Module**: For data storage.
- **Button**: For triggering SMS alerts.

