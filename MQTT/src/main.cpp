#include <Arduino.h>
#include <WiFi.h>
#include "CONFIGS.hpp"
#include <PubSubClient.h> 
#include "Adafruit_BME680.h"
#include "NTPClient.h"
#include "WiFiUdp.h"

// Define sea level pressure
#define SEALEVELPRESSURE_HPA 1013.0F

// Create wifi client
WiFiClient espClient;
PubSubClient client(espClient);

// Create BME680 object
Adafruit_BME680 bme;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

//Set static IP
IPAddress ip(192, 168, 1, 250);

// Create struct to store sensor data
typedef struct 
{
    unsigned long time;
    float temperature;
    float humidity;
    float pressure;
    float altitude;
    float gas_resistance;
} Data;

// Declare global variables
static unsigned long lastMsg;
static Data data;

/**
 * @brief Read temperature from BME680 sensor
 */
float readBME680Temperature()
{
    float t = bme.readTemperature();
    static float t_mem;
    if (isnan(t))
    {
        Serial.println("Failed to read temperature from BME680 sensor ! Kept the old value");
        return t_mem;
    }
    t_mem = t;
    // Serial.println("Read temperature : " + String(t) + "°C");
    return t;
}

/**
 * @brief Read humidity from BME680 sensor
 */
float readBME680Humidity()
{
    float h = bme.readHumidity();
    static float h_mem;
    if (isnan(h))
    {
        Serial.println("Failed to read humidity from BME680 sensor ! Kept the old value");
        return h_mem;
    }
    h_mem = h;
    // Serial.println("Read humidity : " + String(h) + "°C");
    return h;
}

/**
 * @brief Read pressure from BME680 sensor
 */
float readBME680Pressure()
{
    float p = bme.readPressure() / 100.0F;
    static float p_mem;
    if (isnan(p))
    {
        Serial.println("Failed to read pressure from BME680 sensor ! Kept the old value");
        return p_mem;
    }
    p_mem = p;
    // Serial.println("Read pressure : " + String(p) + "hPa");
    return p;
}

/**
 * @brief Read altitude from BME680 sensor
 */
float readBME680Altitude()
{
    float a = bme.readAltitude(SEALEVELPRESSURE_HPA);
    static float a_mem;
    if (isnan(a))
    {
        Serial.println("Failed to read altitude from BME680 sensor ! Kept the old value");
        return a_mem;
    }
    a_mem = a;
    // Serial.println("Read altitude : " + String(a) + "m");
    return a;
}

/**
 * @brief Read gas resistance from BME680 sensor
 */
float readBME680GasResistance()
{
    float r = bme.gas_resistance / 1000.0F;
    static float r_mem;
    if (isnan(r))
    {
        Serial.println("Failed to read gas resistance from BME680 sensor ! Kept the old value");
        return r_mem;
    }
    r_mem = r;
    // Serial.println("Read gas resistance : " + String(r) + "kOhm");
    return r;
}

/**
 * @brief Read epoch from NTP server in milliseconds
 */
unsigned long readTime()
{
    timeClient.update();
    return timeClient.getEpochTime();
}

/**
 * @brief Read temperature and humidity from DHT11 sensor
 */
void updateData()
{
    data.time            = readTime();
    data.temperature     = readBME680Temperature();
    data.humidity        = readBME680Humidity();
    data.pressure        = readBME680Pressure();
    data.altitude        = readBME680Altitude();
    data.gas_resistance  = readBME680GasResistance();
}

void setup()
{
    // Initialize variables
    lastMsg = 0;
    memset(&data, 0, sizeof(data));

    // Start serial
    Serial.begin(115200);   

    // Start BME680
    if (!bme.begin())
    {
        Serial.println("Could not find a valid BME680 sensor, check wiring!");
        while (1);
    }

    // Start wifi
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    WiFi.config(ip, WiFi.gatewayIP(), WiFi.subnetMask(), IPAddress(8, 8, 8, 8));
    Serial.println("Connected to WiFi network at " + String(WiFi.localIP()));

    // Start NTP client
    timeClient.begin();
    timeClient.setTimeOffset(7200);
    timeClient.setUpdateInterval(1000); 

    client.setServer(MQTT_SERVER, 1883);
}

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect("ESP32client"))
        {
            Serial.println("connected");
            // Subscribe
            client.subscribe("test_channel");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}


void loop()
{
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();

    unsigned long now = millis();
    if (millis() - lastMsg > 5000)
    {
        lastMsg = now;

        updateData();

        Serial.println("Sending message to MQTT Server...");
        
        // Fill json string to send with value from struct
        char json[128] = "";
        strncat(json, "{\"time\" : ", 12);
        strncat(json, String(data.time).c_str(), strlen(String(data.time).c_str()));
        strncat(json, ", \"temperature\" : ", 20);
        strncat(json, String(data.temperature).c_str(), strlen(String(data.temperature).c_str()));
        strncat(json, ", \"humidity\" : ", 17);
        strncat(json, String(data.humidity).c_str(), strlen(String(data.humidity).c_str()));
        strncat(json, "}", 2);        

        client.publish("test_channel", json, strlen(json));
        Serial.print("Sent : ");
        Serial.println(json);
    }
}