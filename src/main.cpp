#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Update.h>
#include "config.h"
#include "secrets.h"
#include "temp_hum_sim.h"

WiFiClientSecure wifi;
HttpClient client(wifi, THINGSBOARD_HOST_RAW, 443);

unsigned long lastSensorRead = 0;

struct SystemStatus
{
    int soil_moisture_percentage;
    float temperature;
    float humidity;
    int light_level;
};

SystemStatus systemStatus = {
    .soil_moisture_percentage = 0,
    .temperature = 0.0,
    .humidity = 0.0,
    .light_level = 0};

void setup()
{
    Serial.begin(115200);

    analogReadResolution(12);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    wifi.setInsecure();
    wifi.setTimeout(HTTPS_TIMEOUT);

    pinMode(PHOTO_TRANSISTOR_PIN, INPUT);
    pinMode(SOIL_MOISTURE_PIN, INPUT);

    Wire.begin();

    Serial.println("Initialization complete");
}

void readSensors()
{
    systemStatus.temperature = simulateTemperature();
    systemStatus.humidity = simulateHumidity();

    int lightRaw = analogRead(PHOTO_TRANSISTOR_PIN);
    systemStatus.light_level = (lightRaw * 100) / 4095;

    int moistureRaw = analogRead(SOIL_MOISTURE_PIN);
    if (moistureRaw > MOISTURE_AIR_VALUE)
        moistureRaw = MOISTURE_AIR_VALUE;
    if (moistureRaw < MOISTURE_WATER_VALUE)
        moistureRaw = MOISTURE_WATER_VALUE;
    systemStatus.soil_moisture_percentage =
        ((MOISTURE_AIR_VALUE - moistureRaw) * 100) / (MOISTURE_AIR_VALUE - MOISTURE_WATER_VALUE);

    Serial.printf("Sensors - T:%.1f°C H:%.1f%% L:%d%% M:%d%%\n",
                  systemStatus.temperature, systemStatus.humidity,
                  systemStatus.light_level, systemStatus.soil_moisture_percentage);
}

void sendToThingsBoard()
{
    static int failureCount = 0;
    const int MAX_FAILURES = 3;
    static unsigned long lastConnectionAttempt = 0;

    unsigned long currentTime = millis();
    if (currentTime - lastConnectionAttempt < RETRY_INTERVAL)
    {
        return;
    }
    lastConnectionAttempt = currentTime;

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected");
        return;
    }

    StaticJsonDocument<200> doc;
    // doc["brightness_sensor"] = systemStatus.light_level;
    // doc["humidity_sensor"] = systemStatus.humidity;
    // doc["moisture_sensor"] = systemStatus.soil_moisture_percentage;
    // doc["temperature_sensor"] = systemStatus.temperature;

    doc["brightness_sensor"] = 20;
    doc["humidity_sensor"] = 60;
    doc["moisture_sensor"] = 40;
    doc["temperature_sensor"] = 17;

    String jsonString;
    serializeJson(doc, jsonString);

    Serial.println("Sending data to ThingsBoard...");
    Serial.println(jsonString);

    client.beginRequest();
    client.post("/api/v1/" ACCESS_TOKEN "/telemetry");
    client.sendHeader("Content-Type", "application/json");
    client.sendHeader("Content-Length", jsonString.length());
    client.beginBody();
    client.print(jsonString);
    client.endRequest();

    int statusCode = client.responseStatusCode();

    if (statusCode == 200)
    {
        Serial.println("Data sent to ThingsBoard successfully.");
        failureCount = 0;
    }
    else
    {
        Serial.printf("Failed to send data. Status code: %d\n", statusCode);
        failureCount++;

        if (failureCount >= MAX_FAILURES)
        {
            Serial.println("Too many failures, resetting connection...");
            client.stop();
            wifi.stop();
            delay(1000);

            wifi.setInsecure();
            wifi.setTimeout(HTTPS_TIMEOUT);
            failureCount = 0;
        }
    }

    client.stop();
}

void loop()
{
    unsigned long currentMillis = millis();

    if (currentMillis - lastSensorRead >= SENSOR_INTERVAL)
    {
        readSensors();
        sendToThingsBoard();
        lastSensorRead = currentMillis;
    }
}