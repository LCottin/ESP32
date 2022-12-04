#include <Arduino.h>
#include <WiFi.h>
#include "CONFIGS.hpp"

void setup() 
{
    Serial.begin(115200);
    Serial.println("Hello World!");
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("Connected to the WiFi network");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC address: ");
    Serial.println(WiFi.macAddress());
}

void loop() 
{
}