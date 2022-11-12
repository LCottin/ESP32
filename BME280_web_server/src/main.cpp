#include <Arduino.h>
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include "DHT.h"
#include "CONFIGS.hpp"
#include "Adafruit_BME280.h"
#include "NTPClient.h"
#include "WiFiUdp.h"

#define DHTPIN               4
#define DHTTYPE              DHT11
#define LED                  2
#define SEALEVELPRESSURE_HPA 1025.0F

// Create DHT object
DHT dht(DHTPIN, DHTTYPE);

// Create BME280 object
Adafruit_BME280 bme;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Create struct to store sensor data
typedef struct 
{
    unsigned long time;

    struct dht11_data
    {
        float temperature;
        float humidity;
    } dht11;

    struct bme280_data
    {
        float temperature;
        float humidity;
        float pressure;
        float altitude;
    } bme280;
} Data;
Data data;

/**
 * @brief Read temperature from DHT11 sensor
 */
float readDHTTemperature()
{
    float t = dht.readTemperature();
    static float t_mem;
    if (isnan(t))
    {
        Serial.println("Failed to read temperature from DHT sensor ! Kept the old value");
        return t_mem;
    }
    t_mem = t;
    Serial.println("Read temperature : " + String(t) + "°C");
    return t;
}

/**
 * @brief Read humidity from DHT11 sensor
 */
float readDHTHumidity()
{
    float h = dht.readHumidity();
    static float h_mem;
    if (isnan(h))
    {
        Serial.println("Failed to read humidity from DHT sensor ! Kept the old value");
        return h_mem;
    }
    h_mem = h;
    Serial.println("Read humidity : " + String(h) + "°C");
    return h;
}

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
    Serial.println("Read humidity : " + String(h) + "°C");
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
    Serial.println("Read altitude : " + String(a) + "m");
    return a;
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
    data.time               = readTime();
    data.dht11.temperature  = readDHTTemperature();
    data.dht11.humidity     = readDHTHumidity();
    data.bme280.temperature = readBME280Temperature();
    data.bme280.humidity    = readBME280Humidity();
    data.bme280.pressure    = readBME280Pressure();
    data.bme280.altitude    = readBME280Altitude();
}

/**
 * @brief Convert the structure to a string where each value is separated by a space
 */
String structToString()
{
    String str = "";

    str += String(data.time);
    str += " ";
    str += String(data.dht11.temperature);
    str += " ";
    str += String(data.dht11.humidity);
    str += " ";
    str += String(data.bme280.temperature);
    str += " ";
    str += String(data.bme280.humidity);
    str += " ";
    str += String(data.bme280.pressure);
    str += " ";
    str += String(data.bme280.altitude);
    str += "\n";

    Serial.println("String to send : " + str);
    return str;
}

/**
 * @brief Blink LED 5 times when esp32 is ready
 */
inline void blinkWhenReady()
{
    for (byte i = 0; i < 5; i++)
    {
        digitalWrite(LED, HIGH);
        delay(100);
        digitalWrite(LED, LOW);
        delay(100);
    }
}

void setup()
{
    // Initialize variables
    unsigned char attempts  = 0;
    bool led_on             = false;
    memset(&data, 0, sizeof(data));

    // Initialize serial 
    Serial.begin(115200);
    pinMode(LED, OUTPUT);

    // Initialize SPIFFS
    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    // Initialize DHT sensor
    dht.begin();
    delay(1000);

    // Initialize BME280 sensor
    if (!bme.begin(0x76))
    {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        return;
    }

    // Connect to Wi-Fi
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi.. Attempt " + String(++attempts));
        digitalWrite(LED, led_on);
        led_on = !led_on;
        if (attempts > 10)
        {
            Serial.println("Failed to connect to WiFi");
            ESP.restart();
        }
    }

    // Print ESP32 Local IP Address
    Serial.println(WiFi.localIP());

    // Initialize NTP client
    timeClient.begin();
    timeClient.setTimeOffset(0);
    timeClient.setUpdateInterval(1000);

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(SPIFFS, "/index.html", String()); 
    });

    // Route for style.css file
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(SPIFFS, "/style.css", "text/css"); 
    });

    // Route for js file
    server.on("/function.js", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(SPIFFS, "/function.js", "text/javascript"); 
    });

    // Read sensors
    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(200, "text/plain", structToString().c_str());
    });

    // Start server
    server.begin();

    // Light the LED when connected
    blinkWhenReady();
}

void loop()
{
    updateData();
    delay(1000);
}