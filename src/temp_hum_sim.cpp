#include "temp_hum_sim.h"

int getCurrentMonth() {
    // __DATE__ format is: "Mmm DD YYYY" (e.g., "Jan 27 2024")
    const char* compilationDate = __DATE__;
    const char* monthStr = compilationDate; // First three chars are month
    
    // Convert month abbreviation to number (1-12)
    if(strncmp(monthStr, "Jan", 3) == 0) return 1;
    if(strncmp(monthStr, "Feb", 3) == 0) return 2;
    if(strncmp(monthStr, "Mar", 3) == 0) return 3;
    if(strncmp(monthStr, "Apr", 3) == 0) return 4;
    if(strncmp(monthStr, "May", 3) == 0) return 5;
    if(strncmp(monthStr, "Jun", 3) == 0) return 6;
    if(strncmp(monthStr, "Jul", 3) == 0) return 7;
    if(strncmp(monthStr, "Aug", 3) == 0) return 8;
    if(strncmp(monthStr, "Sep", 3) == 0) return 9;
    if(strncmp(monthStr, "Oct", 3) == 0) return 10;
    if(strncmp(monthStr, "Nov", 3) == 0) return 11;
    if(strncmp(monthStr, "Dec", 3) == 0) return 12;
    
    return 1; // Default to January if something goes wrong
}

float simulateTemperature() {
    // Get current month (1-12)
    int currentMonth = getCurrentMonth();
    
    // Get current hour (0-23)
    // Using millis() to simulate time of day since boot
    int currentHour = (millis() / (1000 * 60 * 60)) % 24;
    
    // Define seasonal base temperature ranges for Madrid
    float minTemp, maxTemp;
    
    // Winter (December-February)
    if (currentMonth == 12 || currentMonth <= 2) {
        minTemp = -2.0;  // Pre-dawn winter minimum
        maxTemp = 12.0;  // Winter afternoon maximum
    }
    // Spring (March-May)
    else if (currentMonth >= 3 && currentMonth <= 5) {
        minTemp = 5.0;   // Spring dawn minimum
        maxTemp = 22.0;  // Spring afternoon maximum
    }
    // Summer (June-August)
    else if (currentMonth >= 6 && currentMonth <= 8) {
        minTemp = 15.0;  // Summer night minimum
        maxTemp = 35.0;  // Summer afternoon maximum
    }
    // Autumn (September-November)
    else {
        minTemp = 8.0;   // Autumn dawn minimum
        maxTemp = 25.0;  // Autumn afternoon maximum
    }
    
    // Apply daily temperature curve
    // Coolest at 5AM, warmest at 3PM (15:00)
    float tempRange = maxTemp - minTemp;
    if (currentHour >= 5 && currentHour < 15) {
        // Temperature rising (5AM to 3PM)
        float progressToMax = (currentHour - 5) / 10.0;  // 0.0 to 1.0
        minTemp += tempRange * progressToMax;
        maxTemp -= tempRange * (1.0 - progressToMax) * 0.1;  // Keep max temp slightly variable
    } else {
        // Temperature falling (3PM to 5AM)
        float progressToMin = (currentHour < 5 ? (currentHour + 24 - 15) : (currentHour - 15)) / 14.0;  // 0.0 to 1.0
        maxTemp -= tempRange * progressToMin;
        minTemp += tempRange * (1.0 - progressToMin) * 0.1;  // Keep min temp slightly variable
    }
    
    // Convert to integer range for random() function
    int minTempInt = minTemp * 10;
    int maxTempInt = maxTemp * 10;
    
    // Generate temperature with daily variation
    return random(minTempInt, maxTempInt) / 10.0;
}

float simulateHumidity() {
    // Get current month (1-12)
    int currentMonth = getCurrentMonth();
    
    // Get current hour (0-23)
    int currentHour = (millis() / (1000 * 60 * 60)) % 24;
    
    // Define seasonal humidity ranges for Madrid
    float minHumidity, maxHumidity;
    
    // Winter (December-February)
    if (currentMonth == 12 || currentMonth <= 2) {
        minHumidity = 65.0;  // Winter day minimum
        maxHumidity = 85.0;  // Winter night maximum
    }
    // Spring (March-May)
    else if (currentMonth >= 3 && currentMonth <= 5) {
        minHumidity = 45.0;  // Spring day minimum
        maxHumidity = 75.0;  // Spring night maximum
    }
    // Summer (June-August)
    else if (currentMonth >= 6 && currentMonth <= 8) {
        minHumidity = 25.0;  // Summer day minimum
        maxHumidity = 45.0;  // Summer night maximum
    }
    // Autumn (September-November)
    else {
        minHumidity = 40.0;  // Autumn day minimum
        maxHumidity = 70.0;  // Autumn night maximum
    }
    
    // Apply daily humidity curve
    // Highest at dawn (5AM), lowest at mid-afternoon (3PM)
    float humidityRange = maxHumidity - minHumidity;
    if (currentHour >= 5 && currentHour < 15) {
        // Humidity falling (5AM to 3PM)
        float progressToMin = (currentHour - 5) / 10.0;  // 0.0 to 1.0
        maxHumidity -= humidityRange * progressToMin;
        minHumidity += humidityRange * (1.0 - progressToMin) * 0.1;
    } else {
        // Humidity rising (3PM to 5AM)
        float progressToMax = (currentHour < 5 ? (currentHour + 24 - 15) : (currentHour - 15)) / 14.0;  // 0.0 to 1.0
        minHumidity += humidityRange * progressToMax;
        maxHumidity -= humidityRange * (1.0 - progressToMax) * 0.1;
    }
    
    // Convert to integer range for random() function
    int minHumidityInt = minHumidity * 10;
    int maxHumidityInt = maxHumidity * 10;
    
    // Generate humidity with daily variation
    return random(minHumidityInt, maxHumidityInt) / 10.0;
}