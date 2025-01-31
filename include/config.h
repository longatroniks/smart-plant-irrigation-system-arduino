#ifndef CONFIG_H
#define CONFIG_H

// Pin definitions
#define PHOTO_TRANSISTOR_PIN 32
#define SOIL_MOISTURE_PIN 33

// Soil moisture calibration values
#define MOISTURE_AIR_VALUE 2450
#define MOISTURE_WATER_VALUE 1150

// Networking configuration
#define HTTPS_TIMEOUT 10000 // HTTPS connection timeout in milliseconds
#define RETRY_INTERVAL 10000 // Retry interval for failed connections in milliseconds

// Sensor and pump intervals
#define SENSOR_INTERVAL 2000 // Interval for reading sensors in milliseconds

#endif // CONFIG_H
