#include <Arduino.h>
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include "DHT.h"
#include "CONFIGS.hpp"

#define DHTPIN 14
#define DHTTYPE DHT11

// Create DHT object
DHT dht(DHTPIN, DHTTYPE);

// Wifi settings
const char led       = 2;
const char *ssid     = SSID;
const char *password = PASSWORD;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

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

// Replace placeholder with DHT values
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
    Serial.begin(115200);
    pinMode(led, OUTPUT);

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
    unsigned attempts = 0;
    bool led_on = false;
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi.. Attempt " + String(++attempts));
        digitalWrite(led, led_on);
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
        request->send(SPIFFS, "/index.html", String(), false, processor); 
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

    // Read temperature
    server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(200, "text/plain", readDHTTemperature().c_str());
    });

    // Read humidity
    server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(200, "text/plain", readDHTHumidity().c_str());
    });

    // Start server
    server.begin();

    // Light the LED when connected
    digitalWrite(led, HIGH);
}

void loop()
{
}