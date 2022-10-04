#include <Arduino.h>
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include "DHT.h"
#include "CONFIGS.hpp"
#include <vector>

#define DHTPIN  14
#define DHTTYPE DHT11
#define LED     2

using namespace std;

// Create DHT object
DHT dht(DHTPIN, DHTTYPE);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create struct to store sensor data
typedef struct 
{
    float temperature;
    float humidity;
} Data;
vector<Data> data(10);
uint16_t data_index;

/**
 * @brief Read temperature from DHT11 sensor
 * @return String containing temperature
 */
String readDHTTemperature()
{
    float t = dht.readTemperature();
    static float t_mem;
    if (isnan(t))
    {
        Serial.println("Failed to read temperature from DHT sensor ! Kept the old value");
        return String(t_mem);
    }
    t_mem = t;
    Serial.println("Read temperature : " + String(t) + "°C");
    return String(t);
}

/**
 * @brief Read humidity from DHT11 sensor
 * @return String containing humidity
 */
String readDHTHumidity()
{
    float h = dht.readHumidity();
    static float h_mem;
    if (isnan(h))
    {
        Serial.println("Failed to read humidity from DHT sensor ! Kept the old value");
        return String(h_mem);
    }
    h_mem = h;
    Serial.println("Read humidity : " + String(h) + "°C");
    return String(h);
}

/**
 * @brief Read temperature and humidity from DHT11 sensor
 * @return String containing temperature and humidity separated by a space
 */
String computeData()
{
    Data d;
    d.temperature    = dht.readTemperature();
    d.humidity       = dht.readHumidity();
    data[data_index] = d;
    data_index++; // Cannot exceed UINT16_MAX, since data is a vector of size UINT16_MAX

    return String(d.temperature) + " " + String(d.humidity);
}

/**
 * @brief Replace placeholders in HTML file
 * @param var placeholder to replace
 * @return String containing the value to replace the placeholder
 */
String processor(const String &var)
{
    if (var == "TEMPERATURE")
    {
        return readDHTTemperature();
    }
    else if (var == "HUMIDITY")
    {
        return readDHTHumidity();
    }
    return String("--");
}

void setup()
{
    // Initialize variables
    unsigned char attempts  = 0;
    bool led_on             = false;
    data_index              = 0;  

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
    delay(2000);

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
    
    // Print chart in another page
    server.on("/chart", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(SPIFFS, "/chart.html", String());
    });

    // Read temperature and humidity
    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(200, "text/plain", computeData().c_str());
    });

    // Start server
    server.begin();

    // Light the LED when connected
    digitalWrite(LED, HIGH);
}

void loop()
{
}