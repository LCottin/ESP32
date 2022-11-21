#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include "CONFIGS.hpp"
#include "Adafruit_BME280.h"
#include "NTPClient.h"
#include "WiFiUdp.h"

#define LED                  2
#define SEALEVELPRESSURE_HPA 1025.0F
#define MY_ID                BEDROOM // ID of the room

// Mac address of the receiver and sender
uint8_t senderAddress[]   = {0xC8, 0xF0, 0x9E, 0xA3, 0x52, 0xA8};
uint8_t receiverAddress[] = {0xC8, 0xF0, 0x9E, 0xA3, 0x53, 0xCC};

// Stores id of the rooms
enum ID 
{
    BEDROOM, 
    LIVING_ROOM
};

// Message to send or receive
typedef struct  
{
    uint8_t id;
    float temperature;
    float humidity;
    float pressure;
    float altitude;
    unsigned long time;
} Message;
Message message;

// Create BME280 object
Adafruit_BME280 bme;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Create struct for peer info
esp_now_peer_info_t peerInfo;

/**
 * @brief Get the Wi Fi Channel object
 */
int32_t getWiFiChannel() 
{
    short networks = WiFi.scanNetworks();
    if (networks > 0) 
    {
        for (int i = 0; i < networks; i++) 
        {
            if (WiFi.SSID(i) == WIFI_SSID)
            {
                return WiFi.channel(i);
            }
        }
    }
    return -1;
}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{
    Serial.print("Last Packet Send Status :\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

float readTemperature()
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

float readHumidity()
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

float readPressure()
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

float readAltitude()
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
 * @brief Read temperature and humidity from BME280 sensor
 */
void updateData()
{
    // Read temperature, humidity, pressure and altitude from BME280 sensor
    message.temperature = readTemperature();
    message.humidity    = readHumidity();
    message.pressure    = readPressure();
    message.altitude    = readAltitude();
    message.time        = readTime();
}

/**
 * @brief Send data to receiver
 */ 
void sendData()
{
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(receiverAddress, (uint8_t *) &message, sizeof(message));
    if (result == ESP_OK)
    {
        Serial.println("Data sent successfully");
    }
    else
    {
        Serial.println("Error sending the data : " + String(result));
    }
}
 
void setup() 
{
    // Initialize variables
    unsigned char attempts  = 0;
    bool led_on             = false;
    message.id              = MY_ID;

    // Init Serial Monitor
    Serial.begin(115200);

    // Set channel
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(getWiFiChannel(), WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);
    
    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) 
    {
        Serial.println("Error initializing ESP-NOW");
        delay(2000);
        ESP.restart();
    }

    // Register peer
    memcpy(peerInfo.peer_addr, receiverAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    
    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Failed to add peer");
        delay(2000);
        ESP.restart();
    }

    esp_now_register_send_cb(OnDataSent);

    // Set LED pin as output
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);
}
 
void loop() 
{
    // Update data
    updateData();

    // Send data
    sendData();

    // Wait 2 seconds
    delay(2000);
}