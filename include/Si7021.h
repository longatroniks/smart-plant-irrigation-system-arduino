#ifndef SI7021_H
#define SI7021_H

#include <Wire.h> // Include Wire library for I2C communication

// Si7021 sensor definitions
#define SI7021_ADDR 0x40
#define SI7021_CMD_READ_TEMP 0xE3
#define SI7021_CMD_READ_HUM 0xE5

// Si7021 class declaration
class Si7021 {
public:
    bool begin();                  // Initialize the sensor
    float readTemperature();       // Read temperature in Celsius
    float readHumidity();          // Read relative humidity in percentage

private:
    uint16_t readRawData(uint8_t command); // Helper function to read raw sensor data
};

#endif // SI7021_H
