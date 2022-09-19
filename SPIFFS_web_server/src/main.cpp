#include <Arduino.h>
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include "CONFIGS.hpp"

const char  led      = 2;
const char* ssid     = SSID;
const char* password = PASSWORD;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void setup()
{
    Serial.begin(115200);
    pinMode(led, OUTPUT);

    bool led_on = false;

    // Initialize SPIFFS
    if(!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    // Connect to Wi-Fi 
    unsigned attempts = 0;
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(1000);
        Serial.println("Connecting to WiFi.. Attempt " + String(++attempts));
        digitalWrite(led, led_on);
        led_on = !led_on;
    }

    // Light the LED when connected
    digitalWrite(led, HIGH);

    // Print ESP32 Local IP Address
    Serial.println(WiFi.localIP());

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(SPIFFS, "/index.html", String(), false);
    });

    // Start server
    server.begin();
}
 
void loop()
{  
}