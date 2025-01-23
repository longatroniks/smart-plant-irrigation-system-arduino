#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include "Si7021.h"
#include <Update.h>

#define PUMP_GPIO_PIN 25
#define PHOTO_TRANSISTOR_PIN 32
#define SOIL_MOISTURE_PIN 33

#define MOISTURE_AIR_VALUE 2450
#define MOISTURE_WATER_VALUE 1150

const char *THINGSBOARD_HOST = "https://srv-iot.diatel.upm.es";
const char *ACCESS_TOKEN = "8lpRi4s20USJgwDYKX0A";
const char *THINGSBOARD_URL = "https://srv-iot.diatel.upm.es/api/v1/8lpRi4s20USJgwDYKX0A/telemetry";

const char *ssid = "Karlo’s iPhone";
const char *password = "kkkkkkkk";

WiFiClientSecure wifi;
HttpClient client(wifi, "srv-iot.diatel.upm.es", 443);

const unsigned long HTTPS_TIMEOUT = 10000;
const unsigned long RETRY_INTERVAL = 10000;

unsigned long lastSensorRead = 0;
unsigned long lastPumpControl = 0;
const unsigned long SENSOR_INTERVAL = 2000;
const unsigned long PUMP_INTERVAL = 1000;

struct SystemStatus
{
    int water_tank_level;
    int soil_moisture_percentage;
    float temperature;
    float humidity;
    int light_level;
    String pump_state;
};

SystemStatus systemStatus = {
    .water_tank_level = 50,
    .soil_moisture_percentage = 0,
    .temperature = 0.0,
    .humidity = 0.0,
    .light_level = 0,
    .pump_state = "OFF"};

Si7021 si7021;

void setup()
{
    Serial.begin(115200);

    analogReadResolution(12);

    WiFi.begin(ssid, password);
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

    pinMode(PUMP_GPIO_PIN, OUTPUT);
    digitalWrite(PUMP_GPIO_PIN, LOW);
    pinMode(PHOTO_TRANSISTOR_PIN, INPUT);
    pinMode(SOIL_MOISTURE_PIN, INPUT);

    Wire.begin();

    if (!si7021.begin())
    {
        Serial.println("Si7021 initialization failed!");
        while (1)
            ;
    }

    randomSeed(analogRead(36));
    systemStatus.water_tank_level = random(40, 61);

    Serial.println("Initialization complete");
}

void readSensors()
{
    systemStatus.temperature = si7021.readTemperature();
    systemStatus.humidity = si7021.readHumidity();

    int lightRaw = analogRead(PHOTO_TRANSISTOR_PIN);
    systemStatus.light_level = (lightRaw * 100) / 4095;

    int moistureRaw = analogRead(SOIL_MOISTURE_PIN);
    if (moistureRaw > MOISTURE_AIR_VALUE)
        moistureRaw = MOISTURE_AIR_VALUE;
    if (moistureRaw < MOISTURE_WATER_VALUE)
        moistureRaw = MOISTURE_WATER_VALUE;
    systemStatus.soil_moisture_percentage =
        ((MOISTURE_AIR_VALUE - moistureRaw) * 100) / (MOISTURE_AIR_VALUE - MOISTURE_WATER_VALUE);

    Serial.printf("Sensors - T:%.1f°C H:%.1f%% L:%d%% M:%d%% W:%d%% P:%s\n",
                  systemStatus.temperature, systemStatus.humidity,
                  systemStatus.light_level, systemStatus.soil_moisture_percentage,
                  systemStatus.water_tank_level, systemStatus.pump_state.c_str());
}

void controlPump()
{
    bool shouldPump =
        systemStatus.water_tank_level > 20 &&
        systemStatus.soil_moisture_percentage < 50 &&
        systemStatus.temperature >= 12.0f &&
        systemStatus.temperature <= 25.0f &&
        systemStatus.humidity < 90.0f &&
        systemStatus.light_level < 60;

    if (shouldPump && systemStatus.pump_state == "OFF")
    {
        digitalWrite(PUMP_GPIO_PIN, HIGH);
        systemStatus.pump_state = "ON";
        Serial.println("Pump ON - Starting irrigation");
    }
    else if (!shouldPump && systemStatus.pump_state == "ON")
    {
        digitalWrite(PUMP_GPIO_PIN, LOW);
        systemStatus.pump_state = "OFF";
        Serial.println("Pump OFF - Conditions not met");
    }

    if (systemStatus.pump_state == "ON")
    {
        systemStatus.water_tank_level--;
    }
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
    doc["brightness_sensor"] = systemStatus.light_level;
    doc["humidity_sensor"] = systemStatus.humidity;
    doc["moisture_sensor"] = systemStatus.soil_moisture_percentage;
    doc["pump_state"] = systemStatus.pump_state;
    doc["temperature_sensor"] = systemStatus.temperature;
    doc["water_level_sensor"] = systemStatus.water_tank_level;

    String jsonString;
    serializeJson(doc, jsonString);

    Serial.println("Sending data to ThingsBoard...");
    Serial.println(jsonString);

    client.beginRequest();
    client.post("/api/v1/8lpRi4s20USJgwDYKX0A/telemetry");
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

    if (currentMillis - lastPumpControl >= PUMP_INTERVAL)
    {
        controlPump();
        lastPumpControl = currentMillis;
    }
}