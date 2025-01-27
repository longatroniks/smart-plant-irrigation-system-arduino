#ifndef TEMP_HUM_SIM_H
#define TEMP_HUM_SIM_H

#include <Arduino.h>

// Function to get current month from compilation date
int getCurrentMonth();

// Function to simulate temperature based on month and time of day
float simulateTemperature();

// Function to simulate humidity based on month and time of day
float simulateHumidity();

#endif // TEMP_HUM_SIM_H