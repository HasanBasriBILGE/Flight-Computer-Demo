#include "mbed.h"
#include "BMP180.h"
#include "MPU6050.h"
#include "Adafruit_SSD1306.h"
#include <cstdint>
#include <cstdio>
#include "nRF24L01P.h"
#include "FATFileSystem.h"
#include "SDBlockDevice.h"

// I2C Bus Addresses
#define bmp180_addr     0x77
#define ssd1306_addr    0x78
#define mpu6050_addr    0x68
#define compass_addr    0x1E

#define I2C_SDA_PIN     D14
#define I2C_SCL_PIN     D15
#define RST_PIN         D2

#define BUTTON_PIN      BUTTON1
#define MOSI_PIN        D11
#define MISO_PIN        D12
#define SCK_PIN         D13
#define CE_PIN          D6

#define RX_PIN          D0
#define TX_PIN          D1

#define CS_1            D10
#define CS_2            D9

// I2C Initializing
I2C i2c(I2C_SDA_PIN, I2C_SCL_PIN);
DigitalOut rst(RST_PIN);

// nRF24L01+ Initializing
nRF24L01P my_nrf24l01p(MOSI_PIN, MISO_PIN, SCK_PIN, CS_2, CE_PIN);

// SD card and filesystem
SDBlockDevice sd(MOSI_PIN, MISO_PIN, SCK_PIN, CS_1);
FATFileSystem fs("sd");

// Creating Objects
Adafruit_SSD1306_I2c oled(i2c, RST_PIN, ssd1306_addr, 32, 128);
BMP180 bmp180(&i2c);
MPU6050 mpu6050(i2c, mpu6050_addr);

BufferedSerial sim800l(TX_PIN, RX_PIN, 9600);

// Interrupt Declarations
InterruptIn button(BUTTON_PIN);

// Thread for display updates
Thread Thread1;

// Thread for handling SIM800L communication
Thread sim800lThread;
volatile bool sendSMS = false; // Flag to indicate SMS request
const char* phoneNumber = "+1234567890"; // SMS gönderilecek telefon numarası

struct SensorData {
    float temp;
    float altitude;
    bool is_okey = false;
    int16_t accel[3];
};

void ssd1306_handler(SensorData* data) {
    rst = 1;
    ThisThread::sleep_for(5ms);

    oled.begin();
    oled.clearDisplay();

    while (true) {
        oled.clearDisplay();
        oled.setTextCursor(0, 0);

        // Display text with formatted float
        oled.printf("Temp     : %d.%d C'\n", int(data->temp), int(data->temp * 10) % 10);
        oled.printf("Altitude : %d.%d m\n", int(data->altitude), int(data->altitude * 10) % 10);
        oled.printf("Accel: (%d %d %d)\n", data->accel[0], data->accel[1], data->accel[2]);
        oled.printf(data->is_okey ? "ReadyToLaunch\n" : "StandBy\n");

        // Update the display with the buffer content
        oled.display();

        ThisThread::sleep_for(1000ms); // Update every second
    }
}

void calibrateMPU6050(MPU6050 &mpu, float* accelBias) {
    const int numSamples = 100;
    float accelSum[3] = {0, 0, 0};
    int16_t ax, ay, az;

    for (int i = 0; i < numSamples; i++) {
        mpu.getAcceleration(&ax, &ay, &az);
        accelSum[0] += ax;
        accelSum[1] += ay;
        accelSum[2] += az;
        ThisThread::sleep_for(10ms);
    }

    accelBias[0] = accelSum[0] / numSamples;
    accelBias[1] = accelSum[1] / numSamples;
    accelBias[2] = accelSum[2] / numSamples;
}

void logDataToSDCard(SensorData* data) {
    FILE *fp = fopen("/sd/sensor_data.csv", "a");
    if (fp) {
        fprintf(fp, "%d.%d,%d.%d,%d,%d,%d\n", int(data->temp), int(data->temp * 10) % 10,
                                              int(data->altitude), int(data->altitude * 10) % 10,
                                              data->accel[0], data->accel[1], data->accel[2]);
        fclose(fp);
    }
}

void sendSMSM(const char* message) {
    const char* command = "AT+CMGF=1\r"; // Set SMS to text mode
    sim800l.write(command, strlen(command));
    ThisThread::sleep_for(500ms); // Wait for response

    char smsCommand[128];
    snprintf(smsCommand, sizeof(smsCommand), "AT+CMGS=\"%s\"\r", phoneNumber); // Set recipient number
    sim800l.write(smsCommand, strlen(smsCommand));
    ThisThread::sleep_for(500ms); // Wait for response

    sim800l.write(message, strlen(message)); // Send SMS body
    ThisThread::sleep_for(500ms); // Wait for response

    sim800l.write("\x1A", 1); // Send Ctrl+Z to indicate end of message
    ThisThread::sleep_for(500ms); // Wait for response
}

void buttonPressed() {
    sendSMSM("Sensor data updated!"); // Set flag and send SMS
}

void sim800lTask() {
    while (true) {
        if (sendSMS) {
            sendSMSM("Sensor data updated!"); // Send SMS when flag is set
            sendSMS = false; // Reset the flag
        }
        ThisThread::sleep_for(100ms); // Check every 100ms
    }
}

// main() runs in its own thread in the OS
int main() {
    // Structure to hold sensor data
    SensorData data;

    // Initialize the BMP180 sensor
    bmp180.init();

    // Initialize the MPU6050 sensor
    mpu6050.initialize();
    float accelBias[3] = {0, 0, 0};
    calibrateMPU6050(mpu6050, accelBias);

    // Mount the filesystem
    fs.mount(&sd);

    rst = 1;
    ThisThread::sleep_for(50ms);
    rst = 0;
    ThisThread::sleep_for(50ms);

    // Initialize nRF24L01+
    my_nrf24l01p.powerUp();
    my_nrf24l01p.setAirDataRate(NRF24L01P_DATARATE_250_KBPS);
    my_nrf24l01p.setTxAddress(0xE7E7E7E7E7, 5); // Set TX address to match the receiver
    my_nrf24l01p.setRxAddress(0xE7E7E7E7E7, 5); // Set RX address to match the receiver
    my_nrf24l01p.setTransferSize(32); // Set payload size
    my_nrf24l01p.setTransmitMode();

    // Set up the button interrupt
    button.fall(&buttonPressed);

    // Start the display update thread
    Thread1.start(callback(ssd1306_handler, &data));

    // Start the SIM800L communication handling thread
    sim800lThread.start(sim800lTask);

    while (true) {
        int pressure;
        // Mpu6050 Declarations
        int16_t ax, ay, az;
        mpu6050.getAcceleration(&ax, &ay, &az);

        // Apply calibration offsets
        data.accel[0] = ax - accelBias[0];
        data.accel[1] = ay - accelBias[1];
        data.accel[2] = az - accelBias[2];

        // Start temperature measurement
        bmp180.startTemperature();
        ThisThread::sleep_for(5ms); // Ensure measurement completes
        bmp180.getTemperature(&data.temp);

        // Start pressure measurement
        bmp180.startPressure(BMP180::STANDARD);
        ThisThread::sleep_for(8ms); // Ensure measurement completes
        bmp180.getPressure(&pressure);
        data.altitude = 44330 * (1 - pow((pressure / 100920.0), (1 / 5.255)));

        // Transmit sensor data using nRF24L01+
        char txData[32];
        sprintf(txData, "T:%.1f A:%.1f X:%d Y:%d Z:%d", data.temp, data.altitude, data.accel[0], data.accel[1], data.accel[2]);
        my_nrf24l01p.write(NRF24L01P_PIPE_P0, txData, sizeof(txData));

        // Log data to SD card
        logDataToSDCard(&data);

        ThisThread::sleep_for(1000ms); // Adjust as needed based on your update rate
    }
}
