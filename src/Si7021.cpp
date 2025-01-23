#include "Si7021.h"

bool Si7021::begin() {
    Wire.beginTransmission(SI7021_ADDR);
    return (Wire.endTransmission() == 0);
}

float Si7021::readTemperature() {
    uint16_t raw = readRawData(SI7021_CMD_READ_TEMP);
    return ((175.72f * raw) / 65536.0f) - 46.85f;
}

float Si7021::readHumidity() {
    uint16_t raw = readRawData(SI7021_CMD_READ_HUM);
    return ((125.0f * raw) / 65536.0f) - 6.0f;
}

uint16_t Si7021::readRawData(uint8_t command) {
    Wire.beginTransmission(SI7021_ADDR);
    Wire.write(command);
    Wire.endTransmission();
    delay(50);

    Wire.requestFrom(SI7021_ADDR, 2);
    if (Wire.available() < 2) return 0;

    uint16_t raw = Wire.read() << 8 | Wire.read();
    return raw;
}
