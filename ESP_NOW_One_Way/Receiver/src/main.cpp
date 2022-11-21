#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "CONFIGS.hpp"

#define LED                  2

// Mac address of the receiver and sender
uint8_t senderAddress[]   = {0xC8, 0xF0, 0x9E, 0xA3, 0x52, 0xA8};
uint8_t receiverAddress[] = {0xC8, 0xF0, 0x9E, 0xA3, 0x53, 0xCC};

// Stores id of the rooms
enum ID 
{
    BEDROOM, 
    LIVING_ROOM
};

// Message to receive
typedef struct  
{
    uint8_t id;
    float temperature;
    float humidity;
    float pressure;
    float altitude;
    unsigned long time;
} Message;

// string to store the message
const String idToString[] = {"Bedroom", "Living room"};

// callback when data is received
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) 
{
    Message message;
    memcpy(&message, incomingData, sizeof(Message));

    Serial.print("Received from : ");
    for (size_t i = 0; i < 6; i++)
    {
        Serial.print(mac_addr[i], HEX);
        if (i < 5)
            Serial.print(":");
    }

    Serial.println("====================================");
    Serial.print("Time : ");
    Serial.println(message.time);
    Serial.print("ID : ");
    Serial.println(idToString[message.id]);
    Serial.print("Temperature : ");
    Serial.print(message.temperature);
    Serial.println(" °C");
    Serial.print("Humidity : ");
    Serial.print(message.humidity);
    Serial.println(" %");
    Serial.print("Pressure : ");
    Serial.print(message.pressure);
    Serial.println(" hPa");
    Serial.print("Altitude : ");
    Serial.println(message.altitude);
    Serial.println("====================================");
}
 
void setup() 
{
    // Initialize variables
    unsigned char attempts  = 0;
    bool led_on             = false;

    // Init Serial Monitor
    Serial.begin(115200);

    // Set the device as a Station and Soft Access Point simultaneously
    WiFi.mode(WIFI_AP_STA);
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
            delay(2000);
            ESP.restart();
        }
    }
    
    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) 
    {
        Serial.println("Error initializing ESP-NOW");
        delay(2000);
        ESP.restart();
    }

    esp_now_register_recv_cb(OnDataRecv);

    // Set LED pin as output
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);
}
 
void loop() 
{
}