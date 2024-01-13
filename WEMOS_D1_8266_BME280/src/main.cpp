#include <Arduino.h>
#include "Adafruit_BME280.h"

#define SEALEVELPRESSURE_HPA 1025.0F

// Create BME280 object
Adafruit_BME280 bme;

// Create struct to store sensor data
typedef struct bme280_data
{
    float temperature;
    float humidity;
    float pressure;
    float altitude;
} bme280_data;
bme280_data data;

float readBME280Temperature()
{
    float t = bme.readTemperature();
    static float t_mem;
    if (isnan(t))
    {
        Serial.println("Failed to read temperature from BME280 sensor ! Kept the old value");
        return t_mem;
    }
    t_mem = t;
    Serial.println("Read temperature : " + String(t) + "°C");
    return t;
}

float readBME280Humidity()
{
    float h = bme.readHumidity();
    static float h_mem;
    if (isnan(h))
    {
        Serial.println("Failed to read humidity from BME280 sensor ! Kept the old value");
        return h_mem;
    }
    h_mem = h;
    Serial.println("Read humidity : " + String(h) + "%");
    return h;
}

float readBME280Pressure()
{
    float p = bme.readPressure() / 100.0F;
    static float p_mem;
    if (isnan(p))
    {
        Serial.println("Failed to read pressure from BME280 sensor ! Kept the old value");
        return p_mem;
    }
    p_mem = p;
    Serial.println("Read pressure : " + String(p) + "hPa");
    return p;
}

float readBME280Altitude()
{
    float a = bme.readAltitude(SEALEVELPRESSURE_HPA);
    static float a_mem;
    if (isnan(a))
    {
        Serial.println("Failed to read altitude from BME280 sensor ! Kept the old value");
        return a_mem;
    }
    a_mem = a;
    Serial.println("Read altitude : " + String(a) + "m\n");
    return a;
}

/**
 * @brief Read temperature and humidity from DHT11 sensor
 */
void updateData()
{
    data.temperature = readBME280Temperature();
    data.humidity    = readBME280Humidity();
    data.pressure    = readBME280Pressure();
    data.altitude    = readBME280Altitude();
}

void setup()
{
    // Initialize variables
    memset(&data, 0, sizeof(data));

    // Initialize serial
    Serial.begin(115200);

    // Initialize BME280 sensor
    if (!bme.begin(0x76))
    {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        return;
    }
}

void loop()
{
    updateData();
    delay(1000);
}