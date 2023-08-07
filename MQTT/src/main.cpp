#include <Arduino.h>
#include <WiFi.h>
#include "CONFIGS.hpp"
#include <Wire.h>
#include <PubSubClient.h> 

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;

//Set static IP
IPAddress ip(192, 168, 1, 250);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

void setup()
{
    Serial.begin(115200);   

    // Start wifi
    WiFi.config(ip, gateway, subnet);
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

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

    long now = millis();
    if (now - lastMsg > 5000)
    {
        lastMsg = now;

        Serial.println("Sending message to MQTT Server...");
        client.publish("test_channel", "hello from esp32");
    }
}