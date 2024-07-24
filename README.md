# Telemetry System for Flight Computer

This project implements a flight computer system with telemetry capabilities. The system collects and displays telemetry data from various sensors, logs the data to an SD card, and can send SMS alerts through a GSM module.

## Features

- **Telemetry Sensors**: Monitors barometric pressure, temperature, and acceleration.
- **Display**: Real-time telemetry data visualization on an OLED screen.
- **Data Logging**: Records telemetry data to an SD card in CSV format.
- **SMS Alerts**: Sends SMS notifications using the SIM800L GSM module.
- **Wireless Communication**: Prepared for future integration with nRF24L01+ (currently commented out).

## Hardware Components

- **Microcontroller**: [Your microcontroller model, e.g., STM32, Nucleo, etc.]
- **BMP180**: Barometric pressure and temperature sensor.
- **MPU6050**: 6-axis accelerometer and gyroscope sensor.
- **SSD1306**: 128x32 OLED display.
- **SIM800L**: GSM/GPRS module for sending SMS alerts.
- **nRF24L01+**: Wireless transceiver module (not currently used).
- **SD Card Module**: For data storage.
- **Button**: For triggering SMS alerts.
